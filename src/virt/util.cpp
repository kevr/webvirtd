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
#include "util.hpp"
#include <fmt/format.h>
#include <libvirt/libvirt.h>
#include <map>
#include <string>
using namespace webvirt;

std::string virt::uri(const std::string &user)
{
    if (user == "root") {
        return "qemu:///system";
    }
    return fmt::format("qemu+ssh://{}@localhost/session", user);
}

const std::map<int, std::string> STATE_STRINGS {
    { VIR_DOMAIN_NOSTATE, "None" },
    { VIR_DOMAIN_RUNNING, "Running" },
    { VIR_DOMAIN_BLOCKED, "Blocked" },
    { VIR_DOMAIN_PAUSED, "Paused" },
    { VIR_DOMAIN_SHUTDOWN, "Shutdown" },
    { VIR_DOMAIN_SHUTOFF, "Shutoff" },
    { VIR_DOMAIN_CRASHED, "Crashed" },
    { VIR_DOMAIN_PMSUSPENDED, "Suspended" },
};

std::string virt::state_string(int state)
{
    return STATE_STRINGS.at(state);
}
