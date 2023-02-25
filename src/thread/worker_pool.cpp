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
#include <thread/worker_pool.hpp>
#include <util/logging.hpp>

using namespace webvirt::thread;

worker_pool::worker_pool(http::io_context &io)
    : io_(io)
{
}

void worker_pool::start(std::size_t num_threads)
{
    for (std::size_t i = 0; i < num_threads; ++i) {
        auto w = std::make_unique<worker>(io_);
        workers_.emplace_back(std::move(w));
        workers_.back()->start();
        logger::debug(fmt::format("Thread {} launched", i + 1));
    }
}

void worker_pool::join()
{
    std::size_t i = 1;
    for (auto &worker : workers_) {
        worker->join();
        logger::debug(fmt::format("Thread {} stopped", i++));
    }
}
