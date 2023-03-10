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
#ifndef STATE_HPP
#define STATE_HPP

#include <http/io_context.hpp>
#include <singleton.hpp>

namespace webvirt
{

/** Application state */
class state : public singleton<state>
{
public:
    /** Priamry io_context used for socket processing */
    http::io_context io;
};

}; // namespace webvirt

#endif /* STATE_HPP */
