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

int webvirt::println(std::string message)
{
    message.push_back('\n');
    return print(std::move(message));
}

int webvirt::error(std::string message, int return_code)
{
    message = "error: " + message;
    std::cerr << message;
    return return_code;
}

int webvirt::errorln(std::string message, int return_code)
{
    message.push_back('\n');
    return error(std::move(message), return_code);
}
