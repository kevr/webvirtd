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
#include "domain.hpp"
#include "../config.hpp"
#include "util.hpp"
#include <chrono>
#include <fmt/format.h>
#include <thread>
using namespace webvirt;
using namespace virt;

virt::domain::domain(libvirt::domain_ptr ptr)
    : domain_(std::move(ptr))
{
}

virt::domain::domain(const domain &other)
    : domain_(other.domain_)
{
}

virt::domain::domain(domain &&other)
    : domain_(std::move(other.domain_))
{
}

virt::domain &virt::domain::operator=(domain other)
{
    domain_ = std::move(other.domain_);
    return *this;
}

virt::domain::operator bool() const
{
    return domain_ != nullptr;
}

int virt::domain::id() const
{
    return libvirt::ref().virDomainGetID(domain_);
}

std::string virt::domain::name() const
{
    return libvirt::ref().virDomainGetName(domain_);
}

int virt::domain::state() const
{
    int state, reason;
    libvirt::ref().virDomainGetState(domain_, &state, &reason, 0);
    return state;
}

bool virt::domain::autostart() const
{
    int autostart_;
    libvirt::ref().virDomainGetAutostart(domain_, &autostart_);
    return autostart_;
}

void virt::domain::autostart(bool enabled)
{
    libvirt::ref().virDomainSetAutostart(domain_, enabled);
}

std::string virt::domain::metadata(int type, const char *uri,
                                   unsigned int flags)
{
    return libvirt::ref().virDomainGetMetadata(domain_, type, uri, flags);
}

bool virt::domain::metadata(int type, const char *metadata, const char *key,
                            const char *uri, unsigned int flags)
{
    return libvirt::ref().virDomainSetMetadata(
               domain_, type, metadata, key, uri, flags) == 0;
}

std::string virt::domain::xml_desc()
{
    return libvirt::ref().virDomainGetXMLDesc(domain_, 0);
}

pugi::xml_document virt::domain::xml_document()
{
    auto desc = xml_desc();
    pugi::xml_document doc;

    // By not checking load results, we return a blank xml_document on failure.
    // A blank xml_document will produce empty string values for
    // children/attributes read, allowing XML routes to fall through
    // gracefully.
    doc.load_string(desc.c_str());

    return doc;
}

libvirt::block_info_ptr virt::domain::block_info(const std::string &device)
{
    return libvirt::ref().virDomainGetBlockInfo(domain_, device.c_str(), 0);
}

bool virt::domain::start()
{
    return libvirt::ref().virDomainCreate(domain_) == 0;
}

bool virt::domain::shutdown()
{
    // If we don't get to the 'Shutdown' state within some tries...
    // then we sent a shutdown when the guest wasn't ready to be
    // shut down.
    auto &lv = libvirt::ref();
    if (lv.virDomainShutdown(domain_) != 0) {
        return false;
    }

    auto &conf = config::instance();
    double timeout = conf.get<double>("libvirt-shutdown-timeout");

    int state = 0, reason;
    std::chrono::steady_clock clock;
    auto start = clock.now();
    int target_state = (1 << VIR_DOMAIN_SHUTDOWN) | (1 << VIR_DOMAIN_SHUTOFF);
    for (lv.virDomainGetState(domain_, &state, &reason, 0);
         ((1 << state) & target_state) == 0;
         lv.virDomainGetState(domain_, &state, &reason, 0)) {
        auto elapsed = std::chrono::duration<double>(clock.now() - start);
        if (elapsed.count() > timeout) {
            throw std::out_of_range(
                fmt::format("{} second timeout exceeded", timeout));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    timeout = conf.get<double>("libvirt-shutoff-timeout");
    start = clock.now();
    for (lv.virDomainGetState(domain_, &state, &reason, 0);
         ((1 << state) & (1 << VIR_DOMAIN_SHUTOFF)) == 0;
         lv.virDomainGetState(domain_, &state, &reason, 0)) {
        auto elapsed = std::chrono::duration<double>(clock.now() - start);
        if (elapsed.count() > timeout) {
            throw std::out_of_range("15 second shutdown timeout exceeded");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    return true;
}

Json::Value virt::domain::simple_json() const
{
    auto &lv = libvirt::ref();
    int state, reason;
    lv.virDomainGetState(domain_, &state, &reason, 0);

    Json::Value data(Json::objectValue);
    int domain_id = lv.virDomainGetID(domain_);
    data["id"] = domain_id;
    const char *name = lv.virDomainGetName(domain_);
    data["name"] = name;
    data["state"] = Json::Value(Json::objectValue);
    data["state"]["id"] = state;
    data["state"]["string"] = virt::state_string(state);

    return data;
} // LCOV_EXCL_LINE
