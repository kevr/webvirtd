/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
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
    std::stringstream ss;
    ss << "usage: webvirtd [options]\n\n"
       << conf.cmdline_desc_ << std::endl
       << conf.desc_ << std::endl;
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
