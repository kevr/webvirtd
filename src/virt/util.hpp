/* SPDX-License-Identifier: Apache 2.0 */
#ifndef VIRT_UTIL_HPP
#define VIRT_UTIL_HPP

#include <string>

namespace webvirt::virt
{

std::string uri(const std::string &user);
std::string state_string(int state);

}; // namespace webvirt::virt

#endif /* VIRT_UTIL_HPP */
