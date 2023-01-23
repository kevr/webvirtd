/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "util.hpp"
#include "syscaller.hpp"
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

std::filesystem::path webvirt::make_tmpdir()
{
    auto &sys = webvirt::syscaller::instance();
    std::filesystem::path tmpdir("/tmp");

    std::string template_("webvirtd-XXXXXX");

    char *result = sys.mkdtemp(template_.data());
    if (!result) {
        std::string error("mkdtemp errno = " + std::to_string(errno) + " (" +
                          strerror(errno) + ")");
        throw std::runtime_error(error.c_str());
    }
    tmpdir /= result;

    if (sys.mkdir(tmpdir.c_str(), S_IRWXU | S_IRWXG) == -1) {
        std::string error("mkdir errno = " + std::to_string(errno) + " (" +
                          strerror(errno) + ")");
        throw std::runtime_error(error.c_str());
    }

    return tmpdir;
}
