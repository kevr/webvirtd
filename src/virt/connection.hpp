/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef VIRT_CONNECTION_HPP
#define VIRT_CONNECTION_HPP

#include <cstring>
#include <errno.h>
#include <json/json.h>
#include <libvirt/libvirt.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace webvirt::virt
{

class connection
{
private:
    struct free_conn {
        void operator()(virConnectPtr ptr);
    };

private:
    std::shared_ptr<virConnect> conn_ = nullptr;
    int errno_ = 0;

public:
    connection() = default;
    connection(const connection &conn);
    connection(const std::string &uri);

    connection &operator=(const connection &conn);

    connection &connect(const std::string &str);

    std::vector<std::map<std::string, Json::Value>> domains();

    int error();
    const char *strerror();
};

}; // namespace webvirt::virt

#endif /* VIRT_CONNECTION_HPP */
