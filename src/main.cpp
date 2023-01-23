/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "http/connection.hpp"
#include "http/io_service.hpp"
#include "http/server.hpp"
#include "syscaller.hpp"
#include <filesystem>
#include <functional>
#include <grp.h>
#include <iostream>
#include <unistd.h>

int setup_socket(webvirt::syscaller &sys,
                 const std::filesystem::path &socket_path)
{
    std::filesystem::permissions(socket_path,
                                 std::filesystem::perms::owner_all |
                                     std::filesystem::perms::group_all,
                                 std::filesystem::perm_options::replace);

    auto group = sys.getgrnam("shadow");
    if (!group) {
        std::cerr << "error: group 'shadow' not found" << std::endl;
        return 2;
    }

    auto uid = getuid();
    auto gid = group->gr_gid;
    if (sys.chown(socket_path.c_str(), uid, gid) == -1) {
        std::cerr << "error: chown failed (" << errno << ")" << std::endl;
        return 3;
    }

    return 0;
}

int webvirt_main(const char *prog, webvirt::io_service &io,
                 const std::string &socket_path)
{
    auto &sys = webvirt::syscaller::instance();

    auto uid = sys.getuid();
    if (uid != 0) {
        std::cerr << "error: " << prog
                  << " must be executed as root, as this daemon"
                  << " performs actions as other users on the system."
                  << std::endl;
        return 1;
    }

    sys.fs_remove(socket_path);
    webvirt::http::server<webvirt::net::unix> server(io, socket_path);
    if (auto rc = setup_socket(sys, socket_path); rc != 0) {
        return rc;
    }

    server.run();
    return 0;
}

int main(int, const char *argv[])
{
    const std::string socket_path = "socket.sock";
    webvirt::io_service io;
    return webvirt_main(argv[0], io, socket_path);
}
