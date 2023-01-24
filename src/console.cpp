/**
 * Description: Main program entry-point.
 *
 * Copyright (C) 2023 Kevin Morris <kevr@0cost.org>
 * All Rights Reserved.
 **/
#include "syscaller.hpp"
#include "json/value.h"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <functional>
#include <iostream>
#include <json/json.h>
#include <map>
#include <virt/connection.hpp>
#include <virt/util.hpp>
using namespace webvirt;

namespace po = boost::program_options;

void domains()
{
    Json::Value json(Json::arrayValue);

    auto &sys = syscaller::instance();
    auto *passwd = sys.getpwuid(sys.getuid());
    if (passwd) {
        virt::connection conn(virt::uri(passwd->pw_name));
        auto domains_ = conn.domains();
        for (auto &domain : domains_)
            json.append(domain);
    }

    Json::FastWriter writer;
    std::cout << writer.write(json);
}

int main(int argc, const char *argv[])
{
    po::options_description desc("program options");
    auto add_option = desc.add_options();
    add_option("help,h", "print this help message");
    add_option("command", "webvirt command");

    po::positional_options_description pos;
    pos.add("command", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                  .options(desc)
                  .positional(pos)
                  .run(),
              vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    auto command = vm.at("command").as<std::string>();
    using function_t = std::function<void()>;
    std::map<std::string, function_t> callbacks { { "domains", domains } };

    auto it = callbacks.find(command);
    if (it == callbacks.end()) {
        return 1;
    }

    auto fn = it->second;
    fn();

    return 0;
}
