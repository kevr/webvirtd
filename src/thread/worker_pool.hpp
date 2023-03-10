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
#ifndef THREAD_WORKER_POOL_HPP
#define THREAD_WORKER_POOL_HPP

#include <http/connection.hpp>
#include <http/io_context.hpp>
#include <http/types.hpp>
#include <thread/worker.hpp>

#include <memory>
#include <string>
#include <vector>

namespace webvirt::thread
{

/** A webvirt::thread::worker container */
class worker_pool
{
private:
    http::io_context &io_;

    using worker_ptr = std::unique_ptr<worker>;
    std::vector<worker_ptr> workers_;

public:
    /** Construct a worker_pool
     *
     * @param io webvirt::http::io_context
     **/
    worker_pool(http::io_context &);

    /** Start workers in the pool
     *
     * @param num_threads Number of worker threads to start
     **/
    void start(std::size_t);

    /** Join all workers in the pool */
    void join();
};

}; // namespace webvirt::thread

#endif /* THREAD_WORKER_POOL_HPP */
