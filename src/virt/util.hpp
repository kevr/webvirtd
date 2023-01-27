/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef VIRT_UTIL_HPP
#define VIRT_UTIL_HPP

#include <string>

namespace webvirt::virt
{

std::string uri(const std::string &user);
std::string state_string(int state);

}; // namespace webvirt::virt

#endif /* VIRT_UTIL_HPP */
