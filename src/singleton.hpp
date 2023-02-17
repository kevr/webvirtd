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
#ifndef SINGLETON_HPP
#define SINGLETON_HPP

namespace webvirt
{

template <typename derivative>
class singleton
{
private:
    inline static derivative instance_;
    inline static derivative *ptr_ = &instance_;

public:
    static derivative &ref()
    {
        return *ptr_;
    }

    static derivative &change(derivative &ref_)
    {
        ptr_ = &ref_;
        return ref();
    }

    static derivative &reset()
    {
        ptr_ = &instance_;
        return ref();
    }
};

}; // namespace webvirt

#endif /* SINGLETON_HPP */
