/* SPDX-License-Identifier: Apache 2.0 */
#include "app.hpp"
#include <pugixml.hpp>
using namespace webvirt;

app::app(webvirt::io_service &io, const std::filesystem::path &socket_path)
    : io_(io)
    , server_(io_, socket_path.string())
{
    router_.route(R"(^.+[^/]$)", bind(&app::append_trailing_slash));
    router_.route(R"(^/domains/$)",
                  http::router::with_methods(
                      { beast::http::verb::post },
                      http::router::with_user(bind_user(&app::domains))));
    router_.route(R"(^/domains/([^/]+)/$)",
                  http::router::with_methods(
                      { beast::http::verb::post },
                      http::router::with_user(bind_user(&app::domain))));
    router_.route(R"(^/domains/([^/]+)/interfaces/$)",
                  http::router::with_methods({ beast::http::verb::post },
                                             http::router::with_user(bind_user(
                                                 &app::domain_interfaces))));

    server_.on_request([this](auto &, const auto &request, auto &response) {
        return router_.run(request, response);
    });
}

std::size_t app::run()
{
    return server_.run();
}

void app::append_trailing_slash(
    const std::smatch &location,
    const beast::http::request<beast::http::dynamic_body> &,
    beast::http::response<beast::http::string_body> &response)
{
    std::string uri(location[0]);
    uri.push_back('/');
    response.set(beast::http::field::location, uri);
    response.result(beast::http::status::temporary_redirect);
}

void app::domains(const std::string &user, const std::smatch &,
                  const beast::http::request<beast::http::dynamic_body> &,
                  beast::http::response<beast::http::string_body> &response)
{
    // Any response we send is JSON-serialized
    response.set("Content-Type", "application/json");

    Json::Value json(Json::arrayValue);
    virt::connection conn;
    try {
        virt_connect(conn, user, response);
    } catch (const std::runtime_error &) {
        return;
    }

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

void app::domain(const std::string &user, const std::smatch &location,
                 const beast::http::request<beast::http::dynamic_body> &,
                 beast::http::response<beast::http::string_body> &response)
{
    const std::string name(location[1]);

    // Any response we send is JSON-serialized
    response.set("Content-Type", "application/json");

    virt::connection conn;
    try {
        virt_connect(conn, user, response);
    } catch (const std::runtime_error &) {
        return;
    }

    auto domain = conn.domain(name);
    if (domain.get("id", "") == "") {
        domain["detail"] = "Unable to find domain";
        response.result(beast::http::status::not_found);
    }

    auto output = json::stringify(domain);
    response.body().append(output);
    response.content_length(response.body().size());
}

void app::domain_interfaces(
    const std::string &user, const std::smatch &location,
    const beast::http::request<beast::http::dynamic_body> &,
    beast::http::response<beast::http::string_body> &response)
{
    virt::connection conn;
    try {
        virt_connect(conn, user, response);
    } catch (const std::runtime_error &) {
        return;
    }

    const std::string name(location[1]);
    auto desc = conn.xml_desc(name);
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

void app::virt_connect(
    virt::connection &conn, const std::string &user,
    beast::http::response<beast::http::string_body> &response)
{
    Json::Value json(Json::objectValue);
    try {
        conn.connect(virt::uri(user));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what();
        json["detail"] = "Unable to connect to libvirt";
        response.result(beast::http::status::internal_server_error);
        auto output = json::stringify(json);
        response.body().append(output);
        response.content_length(response.body().size());
        throw e;
    }
}
