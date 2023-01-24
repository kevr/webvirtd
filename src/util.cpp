/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "util.hpp"
#include "config.hpp"
#include "syscaller.hpp"
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

std::filesystem::path webvirt::make_tmpdir()
{
    auto &sys = webvirt::syscaller::instance();
    std::string template_("/tmp/webvirtd-XXXXXX");

    char *result = sys.mkdtemp(template_.data());
    if (!result) {
        std::string error("mkdtemp errno = " + std::to_string(errno) + " (" +
                          strerror(errno) + ")");
        throw std::runtime_error(error.c_str());
    }

    return result;
}

std::string webvirt::exec(std::string user, const std::string &cmdline)
{
    auto webvirt_bin = config::instance().get<std::string>("webvirt-binary");
    for (const auto *e : { "'", "\"" }) {
        auto it = user.find(e);
        while (it != std::string::npos) {
            user.replace(it, 1, std::string("\\") + e);
            it = user.find(e);
        }

        it = webvirt_bin.find(e);
        while (it != std::string::npos) {
            webvirt_bin.replace(it, 1, std::string("\\") + e);
            it = webvirt_bin.find(e);
        }
    }

    std::string command("sudo -u '");
    command.append(user);
    command.append("' " + webvirt_bin + " " + cmdline);

    auto &sys = syscaller::instance();
    auto proc = sys.popen(command.c_str(), "r");
    std::string output;
    char buffer[256];
    while (sys.fgets(buffer, 256, proc) != nullptr)
        output.append(buffer);
    sys.pclose(proc);

    return output;
}
