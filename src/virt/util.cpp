/* SPDX-License-Identifier: Apache 2.0 */
#include "util.hpp"
#include <libvirt/libvirt.h>
#include <map>
#include <string>
using namespace webvirt;

std::string virt::uri(const std::string &user)
{
    std::string path(user + "@localhost/session");
    if (user == "root") {
        path = "/system";
    }

    std::string uri("qemu+ssh://");
    uri.append(path);
    return uri;
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
