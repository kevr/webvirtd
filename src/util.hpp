/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef UTIL_HPP
#define UTIL_HPP

#include <filesystem>
#include <iostream>

namespace webvirt
{

std::filesystem::path make_tmpdir();

template <typename T>
void print_stdout(const std::string &heading, const T &value)
{
    std::cout << "*** " << heading << " ***" << std::endl
              << value << std::endl
              << "*** END " << heading << " ***" << std::endl;
}

template <typename T>
void print_request(const T &value)
{
    print_stdout("REQUEST", value);
}

template <typename T>
void print_response(const T &value)
{
    print_stdout("RESPONSE", value);
}

}; // namespace webvirt

#endif /* UTIL_HPP */
