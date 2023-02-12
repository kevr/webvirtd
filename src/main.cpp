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
#include "config.hpp"
#include "http/connection.hpp"
#include "http/io_service.hpp"
#include "http/server.hpp"
#include "syscaller.hpp"
#include <boost/program_options/errors.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <grp.h>
#include <iostream>
#include <unistd.h>

using webvirt::errorln;
using webvirt::print;

int setup_socket(webvirt::syscaller &sys,
                 const std::filesystem::path &socket_path)
{
    std::filesystem::permissions(socket_path,
                                 std::filesystem::perms::owner_all |
                                     std::filesystem::perms::group_all,
                                 std::filesystem::perm_options::replace);

    auto &conf = webvirt::config::instance();
    auto group_str = conf.get<std::string>("socket-group");
    auto *group = sys.getgrnam(group_str.c_str());
    if (!group) {
        return errorln(fmt::format("group '{}' not found", group_str), 2);
    }

    auto uid = sys.getuid();
    auto gid = group->gr_gid;
    if (sys.chown(socket_path.c_str(), uid, gid) == -1) {
        return errorln(fmt::format("chown failed ({})", errno), 3);
    }

    return 0;
}

int webvirt_main(webvirt::io_service &io, const std::string &socket_path)
{
    auto &sys = webvirt::syscaller::instance();

    sys.fs_remove(socket_path);
    webvirt::app app(io, socket_path);
    if (auto rc = setup_socket(sys, socket_path); rc != 0) {
        return rc;
    }

    app.run();
    return 0;
}

int main(int argc, const char *argv[])
{
    FILE *files_[2] = { stdout, stderr };
    for (auto &file : files_) {
        setvbuf(file, nullptr, _IOLBF, 0);
    }

    auto &sys = webvirt::syscaller::instance();

    webvirt::config conf;
    conf.add_option("disable-timestamp", "disable logging timestamps");

    conf.add_option("socket,s",
                    boost::program_options::value<std::string>()
                        ->default_value("/var/run/webvirtd.sock")
                        ->multitoken(),
                    "unix socket path");

    auto gid = sys.getgid();
    auto *default_group = sys.getgrgid(gid);
    conf.add_option("socket-group",
                    boost::program_options::value<std::string>()
                        ->default_value(default_group->gr_name)
                        ->multitoken(),
                    "socket group");

    conf.add_option("libvirt-shutdown-timeout",
                    boost::program_options::value<double>()
                        ->default_value(3.0)
                        ->multitoken(),
                    "timeout in seconds for domain shutdown state to react");

    conf.add_option("libvirt-shutoff-timeout",
                    boost::program_options::value<double>()
                        ->default_value(15.0)
                        ->multitoken(),
                    "timeout in seconds for domain shutoff state to react");

    try {
        conf.parse(argc, argv);
    } catch (const boost::program_options::unknown_option &ec) {
        return errorln(ec.what(), 1);
    }

    if (conf.has("help")) {
        return print(conf);
    }

    std::filesystem::path config_path(conf.get<std::string>("config"));
    conf.parse(config_path);

    // Parse command-line again; those options are prioritized over config
    conf.parse(argc, argv);

    const auto socket_path = conf.get<std::string>("socket");
    webvirt::config::change(conf);
    webvirt::io_service io;
    return webvirt_main(io, socket_path);
}
