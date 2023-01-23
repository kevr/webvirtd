/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "io_service.hpp"

webvirt::stubs::io_service::io_service(std::size_t iterations)
    : webvirt::io_service()
{
    this->iterations = iterations;
}

std::size_t webvirt::stubs::io_service::process()
{
    for (std::size_t i = 0; i < iterations; ++i) {
        this->run_one();
    }
    return iterations;
}