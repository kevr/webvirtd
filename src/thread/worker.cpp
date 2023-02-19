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
#include <thread/worker.hpp>

#include <chrono>
#include <cstdint>
#include <utility>

using namespace webvirt::thread;

worker::worker(http::io_context &io)
    : io_(io)
{
}

worker::~worker()
{
    join();
}

void worker::start()
{
    thread_ = std::thread(std::bind(&worker::loop, this));
}

void worker::join()
{
    if (thread_.joinable()) {
        thread_.join();
    }
}

void worker::loop()
{
    io_.run();
}
