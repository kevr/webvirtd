/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <boost/program_options.hpp>
#include <filesystem>

namespace webvirt
{

class config
{
private:
    boost::program_options::options_description cmdline_desc_ {
        "command-line options"
    };
    boost::program_options::options_description desc_ { "program options" };
    boost::program_options::variables_map vm_;

public:
    config();
    config(const config &);
    config(config &&);
    config(int, const char *[]);
    config(const std::filesystem::path &file);

    config &add_option(const char *name, const char *desc);

    template <typename value_t>
    config &add_option(const char *name, value_t value, const char *desc)
    {
        desc_.add_options()(name, std::move(value), desc);
        return *this;
    }

    bool has(const char *name);

    template <typename value_t>
    value_t get(const char *name)
    {
        return vm_.at(name).as<value_t>();
    }

    config &parse(int, const char *[]);
    config &parse(const std::filesystem::path &file);

    friend std::ostream &operator<<(std::ostream &, const config &);

private:
    void init();
};

std::ostream &operator<<(std::ostream &, const config &);

}; // namespace webvirt

#endif /* CONFIG_HPP */
