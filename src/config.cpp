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
#include "config.hpp"
#include "syscaller.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <sstream>
using namespace webvirt;

config config::instance_;
config *config::ptr_ = &instance_;

config::config()
{
    init();
}

config::config(const config &other)
    : desc_(other.desc_)
    , vm_(other.vm_)
{
}

config::config(config &&other)
    : desc_(std::move(other.desc_))
    , vm_(std::move(other.vm_))
{
}

config::config(int argc, const char *argv[])
    : config()
{
    parse(argc, argv);
}

config::config(const std::filesystem::path &file)
    : config()
{
    parse(file);
}

config &config::add_option(const char *name, const char *desc)
{
    desc_.add_options()(name, desc);
    return *this;
}

bool config::has(const char *name)
{
    return vm_.count(name);
}

config &config::parse(int argc, const char *argv[])
{
    boost::program_options::options_description visible;
    visible.add(cmdline_desc_).add(desc_);
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, visible), vm_);
    return *this;
}

config &config::parse(const std::filesystem::path &file)
{
    if (std::filesystem::exists(file)) {
        boost::program_options::store(
            boost::program_options::parse_config_file(file.c_str(), desc_),
            vm_);
    }
    return *this;
}

std::ostream &webvirt::operator<<(std::ostream &os,
                                  const webvirt::config &conf)
{
    const std::string epilog(
        "configuration\n"
        "  Options found in the 'program options' section can be set both in\n"
        "  configuration files and with command-line usage.\n\n"
        "  Default configuration is located at ~/.webvirtd.conf and can be\n"
        "  overridden using the command-line '--config' option.\n");

    std::stringstream ss;
    ss << "usage: webvirtd [options]\n\n"
       << conf.cmdline_desc_ << std::endl
       << conf.desc_ << std::endl
       << epilog << std::endl;
    os << ss.str();
    return os;
}

config &config::change(config &conf)
{
    ptr_ = &conf;
    return instance();
}

config &config::instance()
{
    return *ptr_;
}

config &config::reset()
{
    ptr_ = &instance_;
    return instance();
}

void config::init()
{
    auto &sys = syscaller::instance();
    std::filesystem::path home(sys.getenv("HOME"));
    home /= ".webvirtd.conf";

    auto add_option = cmdline_desc_.add_options();
    add_option("help,h", "print this help message");
    add_option("config,c",
               boost::program_options::value<std::string>()->default_value(
                   home.string()),
               "path to configuration file");
}
