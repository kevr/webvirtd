/*
 * Copyright 2023 Kevin Morris
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#include "app.hpp"
#include <pugixml.hpp>
using namespace webvirt;

app::app(webvirt::io_service &io, const std::filesystem::path &socket_path)
    : io_(io)
    , server_(io_, socket_path.string())
{
    router_.route(R"(^.+[^/]$)", bind(&app::append_trailing_slash));
    router_.route(
        R"(^/domains/$)",
        http::router::with_methods(
            { beast::http::verb::post },
            http::router::with_libvirt(bind_libvirt(&app::domains))));
    router_.route(R"(^/domains/([^/]+)/$)",
                  http::router::with_methods(
                      { beast::http::verb::post },
                      http::router::with_libvirt(bind_libvirt(&app::domain))));
    router_.route(
        R"(^/domains/([^/]+)/interfaces/$)",
        http::router::with_methods({ beast::http::verb::post },
                                   http::router::with_libvirt(bind_libvirt(
                                       &app::domain_interfaces))));
    router_.route(
        R"(^/domains/([^/]+)/start/$)",
        http::router::with_methods(
            { beast::http::verb::post },
            http::router::with_libvirt(bind_libvirt(&app::domain_start))));
    router_.route(
        R"(^/domains/([^/]+)/shutdown/)",
        http::router::with_methods(
            { beast::http::verb::post },
            http::router::with_libvirt(bind_libvirt(&app::domain_shutdown))));

    server_.on_request([this](auto &, const auto &request, auto &response) {
        return router_.run(request, response);
    });
}

std::size_t app::run()
{
    return server_.run();
}

void app::append_trailing_slash(const std::smatch &location,
                                const http::request &,
                                http::response &response)
{
    std::string uri(location[0]);
    uri.push_back('/');
    response.set(beast::http::field::location, uri);
    response.result(beast::http::status::temporary_redirect);
}

void app::domains(virt::connection &conn, const std::string &,
                  const std::smatch &, const http::request &,
                  http::response &response)
{
    // Any response we send is JSON-serialized
    response.set("Content-Type", "application/json");

    Json::Value json(Json::arrayValue);
    auto domains_ = conn.domains();
    for (auto &domain : domains_) {
        Json::Value map(Json::objectValue);
        for (auto &kv : domain) {
            map[kv.first] = kv.second;
        }
        json.append(map);
    }

    auto output = json::stringify(json);
    response.body().append(output);
    response.content_length(response.body().size());
}

void app::domain(virt::connection &conn, const std::string &,
                 const std::smatch &location, const http::request &,
                 http::response &response)
{
    const std::string name(location[1]);

    // Any response we send is JSON-serialized
    response.set("Content-Type", "application/json");

    auto domain = conn.domain(name);
    if (domain.get("id", "") == "") {
        domain["detail"] = "Unable to find domain";
        response.result(beast::http::status::not_found);
    }

    auto output = json::stringify(domain);
    response.body().append(output);
    response.content_length(response.body().size());
}

void app::domain_interfaces(virt::connection &conn, const std::string &,
                            const std::smatch &location, const http::request &,
                            http::response &response)
{
    const std::string name(location[1]);
    std::string desc;
    try {
        desc = conn.xml_desc(name);
    } catch (const std::domain_error &exc) {
        response.result(beast::http::status::not_found);
    }

    pugi::xml_document doc;
    doc.load_buffer(desc.c_str(), desc.size());

    auto interfaces =
        doc.child("domain").child("devices").children("interface");
    Json::Value json(Json::arrayValue);
    for (auto &interface : interfaces) {
        Json::Value object(Json::objectValue);
        object["macAddress"] =
            interface.child("mac").attribute("address").as_string();
        object["model"] =
            interface.child("model").attribute("type").as_string();
        object["name"] =
            interface.child("alias").attribute("name").as_string();
        json.append(std::move(object));
    }

    response.body().append(json::stringify(json));
    response.content_length(response.body().size());
}

void app::domain_start(virt::connection &conn, const std::string &,
                       const std::smatch &location, const http::request &,
                       http::response &response)
{
    const std::string name(location[1]);
    auto domain = conn.get_domain_ptr(name);
    Json::Value data(Json::objectValue);
    if (!conn.start(domain)) {
        data["detail"] = "Unable to start domain";
        response.result(beast::http::status::bad_request);
        response.body().append(json::stringify(data));
        response.content_length(response.body().size());
        return;
    }

    data = short_json(domain);
    response.result(beast::http::status::created);
    response.body().append(json::stringify(data));
    response.content_length(response.body().size());
}

void app::domain_shutdown(virt::connection &conn, const std::string &,
                          const std::smatch &location, const http::request &,
                          http::response &response)
{
    const std::string name(location[1]);
    auto domain = conn.get_domain_ptr(name);
    Json::Value data(Json::objectValue);

    bool ok = false;
    try {
        ok = conn.shutdown(domain);
    } catch (const std::out_of_range &exc) {
        data["detail"] = "Shutdown operation timed out";
        response.result(beast::http::status::bad_gateway);
        response.body().append(json::stringify(data));
        response.content_length(response.body().size());
        return;
    }

    if (!ok) {
        data["detail"] = "Unable to shutdown domain";
        response.result(beast::http::status::bad_request);
        response.body().append(json::stringify(data));
        response.content_length(response.body().size());
        return;
    }

    data = short_json(domain);
    response.result(beast::http::status::ok);
    response.body().append(json::stringify(data));
    response.content_length(response.body().size());
}

Json::Value app::short_json(libvirt::domain_ptr domain)
{
    auto &lv = libvirt::ref();
    int state, reason;
    lv.virDomainGetState(domain, &state, &reason, 0);

    Json::Value data(Json::objectValue);
    int domain_id = lv.virDomainGetID(domain);
    data["id"] = domain_id;
    const char *name = lv.virDomainGetName(domain);
    data["name"] = name;
    data["state"] = Json::Value(Json::objectValue);
    data["state"]["id"] = state;
    data["state"]["string"] = virt::state_string(state);

    return data;
}
