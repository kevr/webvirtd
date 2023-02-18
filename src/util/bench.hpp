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
#ifndef UTIL_BENCH_HPP
#define UTIL_BENCH_HPP

#include <chrono>

namespace webvirt
{

template <typename value_type>
class bench
{
private:
    std::chrono::high_resolution_clock clock_;
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point end_;

public:
    bench()
    {
        start();
    }

    value_type elapsed() const
    {
        return std::chrono::duration<value_type>(end_ - start_).count();
    }

    void start()
    {
        start_ = clock_.now();
    }

    value_type end()
    {
        end_ = clock_.now();
        return elapsed();
    }
};

}; // namespace webvirt

#endif /* UTIL_BENCH_HPP */
