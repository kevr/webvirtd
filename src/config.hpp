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
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "singleton.hpp"
#include <boost/program_options.hpp>
#include <filesystem>

namespace webvirt
{

class config : public singleton<config>
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
