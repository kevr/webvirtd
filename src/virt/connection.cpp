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
#include "connection.hpp"
#include "util.hpp"
#include <iostream>
#include <pugixml.hpp>
#include <stdexcept>
using namespace webvirt;

void virt::connection::free_conn::operator()(virConnectPtr ptr)
{
    virConnectClose(ptr);
}

virt::connection::connection(const connection &conn)
    : conn_(conn.conn_)
    , errno_(conn.errno_)
{
}

virt::connection::connection(const std::string &uri)
{
    connect(uri);
}

bool virt::connection::operator==(const connection &other) const
{
    return conn_ == other.conn_;
}

bool virt::connection::operator!=(const connection &other) const
{
    return !operator==(other);
}

virt::connection &virt::connection::operator=(const virt::connection &conn)
{
    conn_ = conn.conn_;
    errno_ = conn.errno_;
    return *this;
}

virt::connection &virt::connection::connect(const std::string &str)
{
    if (conn_) {
        throw std::overflow_error("cannot connect more than once");
    }

    auto &lv = libvirt::ref();
    conn_ = lv.virConnectOpen(str.c_str());
    if (!conn_) {
        errno_ = errno;
        std::string message("error: ");
        message.append(this->strerror());
        message.push_back('\n');
        throw std::runtime_error(message);
    }

    return *this;
}

std::vector<std::map<std::string, Json::Value>> virt::connection::domains()
{
    auto &lv = libvirt::ref();
    auto domains = lv.virConnectListAllDomains(conn_, 0);

    std::vector<std::map<std::string, Json::Value>> output;
    for (auto &domain : domains) {
        std::map<std::string, Json::Value> item;

        item["name"] = lv.virDomainGetName(domain);

        int state = 0, reason = 0;
        lv.virDomainGetState(domain, &state, &reason, 0);
        item["state"] = Json::Value(Json::objectValue);
        item["state"]["id"] = state;
        item["state"]["string"] = state_string(state);

        int id = lv.virDomainGetID(domain);
        item["id"] = id;

        output.emplace_back(std::move(item));
    }

    return output;
}

Json::Value virt::connection::domain(const std::string &name)
{
    auto &lv = libvirt::ref();
    Json::Value output(Json::objectValue);
    auto domain = lv.virDomainLookupByName(conn_, name.c_str());
    if (!domain) {
        return output;
    }

    auto desc = xml_desc(domain);

    pugi::xml_document doc;
    doc.load_buffer(desc.c_str(), desc.size());
    auto domain_ = doc.child("domain");

    output["name"] = name;
    output["id"] = domain_.attribute("id").as_int();

    Json::Value info(Json::objectValue);
    info["cpus"] = domain_.child("vcpu").text().as_uint();
    info["maxMemory"] = domain_.child("memory").text().as_uint();
    info["memory"] = domain_.child("currentMemory").text().as_uint();
    info["os"] = domain_.child("metadata")
                     .child("libosinfo:libosinfo")
                     .child("libosinfo:os")
                     .attribute("id")
                     .as_string();

    info["devices"] = Json::Value(Json::objectValue);

    info["devices"]["disks"] = Json::Value(Json::arrayValue);
    auto disks = domain_.child("devices").children("disk");
    for (auto &disk : disks) {
        Json::Value object(Json::objectValue);
        object["device"] = disk.attribute("device").as_string();
        object["driver"] = Json::Value(Json::objectValue);
        object["driver"]["name"] =
            disk.child("driver").attribute("name").as_string();
        object["driver"]["type"] =
            disk.child("driver").attribute("type").as_string();
        object["source"] = Json::Value(Json::objectValue);
        object["source"]["file"] =
            disk.child("source").attribute("file").as_string();
        object["target"] = Json::Value(Json::objectValue);
        object["target"]["dev"] =
            disk.child("target").attribute("dev").as_string();
        object["target"]["bus"] =
            disk.child("target").attribute("bus").as_string();

        info["devices"]["disks"].append(std::move(object));
    }

    info["devices"]["interfaces"] = Json::Value(Json::arrayValue);
    auto interfaces = domain_.child("devices").children("interface");
    for (auto &interface : interfaces) {
        Json::Value object(Json::objectValue);
        object["macAddress"] =
            interface.child("mac").attribute("address").as_string();
        object["model"] =
            interface.child("model").attribute("type").as_string();
        object["name"] =
            interface.child("alias").attribute("name").as_string();
        info["devices"]["interfaces"].append(std::move(object));
    }

    output["info"] = std::move(info);

    int state = 0, reason = 0;
    lv.virDomainGetState(domain, &state, &reason, 0);
    output["state"] = Json::Value(Json::objectValue);
    output["state"]["id"] = state;
    output["state"]["string"] = state_string(state);

    return output;
}

std::string virt::connection::xml_desc(const std::string &name)
{
    auto &lv = libvirt::ref();
    auto domain = lv.virDomainLookupByName(conn_, name.c_str());
    if (!domain) {
        throw std::domain_error("virDomainLookupByName error");
    }

    return xml_desc(domain);
}

std::string virt::connection::xml_desc(libvirt::domain_ptr domain)
{
    auto &lv = libvirt::ref();
    return lv.virDomainGetXMLDesc(domain, 0);
}

int virt::connection::error()
{
    return errno_;
}

const char *virt::connection::strerror()
{
    return ::strerror(error());
}
