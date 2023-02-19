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
#ifndef THREAD_WORKER_HPP
#define THREAD_WORKER_HPP

#include <http/connection.hpp>
#include <http/io_context.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace webvirt::thread
{

class worker
{
private:
    http::io_context &io_;
    std::thread thread_;

public:
    worker(http::io_context &);
    ~worker();

    void start();
    void join();

private:
    void loop();
};

}; // namespace webvirt::thread

#endif /* THREAD_WORKER_HPP */
