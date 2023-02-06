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
#include "domains.hpp"
#include "../json.hpp"
#include "../virt/util.hpp"
using namespace webvirt::views;

void domains::index(virt::connection &conn, const std::string &,
                    const std::smatch &, const http::request &,
                    http::response &response)
{
    // Any response we send is JSON-serialized
    response.set("Content-Type", "application/json");
    auto data = conn.domains();
    response.body().append(json::stringify(std::move(data)));
    response.content_length(response.body().size());
}

void domains::show(virt::connection &conn, const std::string &,
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

void domains::start(virt::connection &conn, const std::string &,
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

    data = simple_domain_json(domain);
    response.result(beast::http::status::created);
    response.body().append(json::stringify(data));
    response.content_length(response.body().size());
}

void domains::shutdown(virt::connection &conn, const std::string &,
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
        response.result(beast::http::status::gateway_timeout);
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

    data = simple_domain_json(domain);
    response.result(beast::http::status::ok);
    response.body().append(json::stringify(data));
    response.content_length(response.body().size());
}

Json::Value domains::simple_domain_json(libvirt::domain_ptr domain)
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
} // LCOV_EXCL_LINE
