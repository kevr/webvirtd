#include "util.hpp"
using namespace webvirt;

std::string virt::uri(const std::string &user)
{
    std::string path("/session");
    if (user == "root") {
        path = "/system";
    }

    std::string uri("qemu://");
    uri.append(path);
    return uri;
}
