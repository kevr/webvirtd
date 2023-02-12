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
#include "libvirt.hpp"
#include <cstring>
#include <string>
using namespace webvirt;

connect *virConnectOpen(const char *)
{
    return nullptr;
}

int virConnectListAllDomains(connect *, domain ***, int)
{
    return 0;
}

int virConnectClose(connect *ptr)
{
    delete ptr;
    return 0;
}

domain *virDomainLookupByName(connect *, const char *)
{
    return nullptr;
}

int virDomainCreate(domain *)
{
    return 0;
}

int virDomainGetState(domain *, int *state, int *, int)
{
    *state = 1;
    return 0;
}

int virDomainGetID(domain *)
{
    return 1;
}

char *virDomainGetName(domain *)
{
    char *buf = static_cast<char *>(std::malloc(5));
    strncpy(buf, "test", 5);
    return buf;
}

int virDomainGetAutostart(domain *, int *)
{
    return 0;
}

int virDomainSetAutostart(domain *, int)
{
    return 0;
}

char *virDomainGetMetadata(domain *, int, const char *, unsigned int)
{
    char *buf = static_cast<char *>(std::malloc(1));
    buf[0] = '\0';
    return buf;
}

int virDomainSetMetadata(domain *, int, const char *, const char *,
                         const char *, unsigned int)
{
    return 0;
}

char *virDomainGetXMLDesc(domain *, int)
{
    char *buf = static_cast<char *>(std::malloc(1));
    buf[0] = '\0';
    return buf;
}

domain *virDomainDefineXML(connect *, const char *)
{
    return new domain;
}

int virDomainGetBlockInfo(domain *, const char *, block_info *, int)
{
    return 0;
}

int virDomainFree(domain *ptr)
{
    delete ptr;
    return 0;
}

int virDomainShutdown(domain *)
{
    return 0;
}
