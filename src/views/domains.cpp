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
#include <data/domain.hpp>
#include <http/util.hpp>
#include <util/json.hpp>
#include <views/domains.hpp>
#include <virt/util.hpp>

#include <boost/cast.hpp>

using namespace webvirt::views;
using namespace std::string_literals;

void domains::index(virt::connection &conn, const std::smatch &,
                    const http::request &, http::response &response)
{
    auto domains = conn.domains();

    Json::Value data(Json::arrayValue);
    for (auto &domain : domains) {
        data.append(data::simple_domain(domain));
    }

    return http::set_response(response, data, beast::http::status::ok);
}

void domains::show(virt::connection &, virt::domain domain,
                   const std::smatch &, const http::request &,
                   http::response &response)
{
    return http::set_response(
        response, data::domain(domain), beast::http::status::ok);
}

void domains::autostart(virt::connection &, virt::domain domain,
                        const std::smatch &, const http::request &request,
                        http::response &response)
{
    bool enabled = request.method() == beast::http::verb::post;
    domain.autostart(enabled);

    Json::Value object(Json::objectValue);
    object["autostart"] = enabled;

    return http::set_response(response, object, beast::http::status::ok);
}

void domains::bootmenu(virt::connection &conn, virt::domain domain,
                       const std::smatch &, const http::request &request,
                       http::response &response)
{
    auto doc = domain.xml_document();
    auto os = doc.child("domain").child("os");
    std::string enabled =
        request.method() == beast::http::verb::post ? "yes" : "no";
    os.remove_child("bootmenu");
    os.append_child("bootmenu");
    os.last_child().append_attribute("enable");
    os.last_child().last_attribute().set_value(enabled.c_str());

    std::stringstream ss;
    doc.save(ss);
    auto xml = ss.str();

    if (!domain.define_xml(conn.get_ptr(), xml.c_str())) {
        return http::set_response(response,
                                  json::error("Unable to replace domain XML"),
                                  beast::http::status::internal_server_error);
    }

    return http::set_response(
        response, json::xml_to_json(os), beast::http::status::ok);
}

void domains::metadata(virt::connection &, virt::domain domain,
                       const std::smatch &, const http::request &request,
                       http::response &response)
{
    Json::Value data(Json::objectValue);
    try {
        data = json::parse(request.body());
    } catch (const std::invalid_argument &) {
        return http::set_response(response,
                                  json::error("Invalid JSON input"),
                                  beast::http::status::bad_request);
    }

    // JSON output from this function.
    Json::Value output(Json::objectValue);

    auto current_title =
        domain.metadata(VIR_DOMAIN_METADATA_TITLE, nullptr, 0);
    auto title = data.get("title", current_title).asString();
    if (title != current_title) {
        domain.metadata(
            VIR_DOMAIN_METADATA_TITLE, title.c_str(), nullptr, nullptr, 0);
        output["title"] = title;
    }

    auto current_desc =
        domain.metadata(VIR_DOMAIN_METADATA_DESCRIPTION, nullptr, 0);
    auto desc = data.get("description", current_desc).asString();
    if (desc != current_desc) {
        domain.metadata(VIR_DOMAIN_METADATA_DESCRIPTION,
                        desc.c_str(),
                        nullptr,
                        nullptr,
                        0);
        output["description"] = desc;
    }

    if (!output.size()) {
        response.result(beast::http::status::not_modified);
    }

    return http::set_response(response, output, response.result());
}

void domains::start(virt::connection &, virt::domain domain,
                    const std::smatch &, const http::request &,
                    http::response &response)
{
    if (!domain.start()) {
        return http::set_response(response,
                                  json::error("Unable to start domain"),
                                  beast::http::status::bad_request);
    }

    return http::set_response(
        response, data::simple_domain(domain), beast::http::status::created);
}

void domains::shutdown(virt::connection &, virt::domain domain,
                       const std::smatch &, const http::request &,
                       http::response &response)
{
    bool ok = false;
    try {
        ok = domain.shutdown();
    } catch (const std::out_of_range &exc) {
        return http::set_response(response,
                                  json::error("Shutdown operation timed out"),
                                  beast::http::status::gateway_timeout);
    }

    if (!ok) {
        return http::set_response(response,
                                  json::error("Unable to shutdown domain"),
                                  beast::http::status::bad_request);
    }

    return http::set_response(
        response, data::simple_domain(domain), beast::http::status::ok);
}
