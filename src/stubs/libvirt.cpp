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
#include <stubs/libvirt.hpp>

#include <cstring>
#include <string>

using namespace webvirt;

static char *make_cstring(const std::string &str)
{
    char *c_str = reinterpret_cast<char *>(malloc(str.size() + 1));
    strncpy(c_str, str.c_str(), str.size() + 1);
    return c_str;
}

connect *virConnectOpen(const char *)
{
    return nullptr;
}

int virConnectRegisterCloseCallback(webvirt::connect *,
                                    void (*)(webvirt::connect *, int, void *),
                                    void *, void (*)(void *))
{
    return 0;
}

char *virConnectGetCapabilities(webvirt::connect *)
{
    return make_cstring("");
}

char *virConnectGetHostname(webvirt::connect *)
{
    return make_cstring("test");
}

int virConnectGetLibVersion(webvirt::connect *, unsigned long *version)
{
    *version = 0;
    return 0;
}

int virConnectGetMaxVcpus(webvirt::connect *, const char *)
{
    return 2;
}

char *virConnectGetSysinfo(webvirt::connect *, unsigned int)
{
    return make_cstring("");
}

const char *virConnectGetType(webvirt::connect *)
{
    return "qemu";
}

char *virConnectGetURI(webvirt::connect *)
{
    return make_cstring("qemu+ssh://test@localhost/session");
}

int virConnectGetVersion(webvirt::connect *, unsigned long *version)
{
    *version = 0;
    return 0;
}

int virConnectIsEncrypted(webvirt::connect *)
{
    return 1;
}

int virConnectIsSecure(webvirt::connect *)
{
    return 1;
}

int virConnectListAllDomains(connect *, domain ***, int)
{
    return 0;
}

int virConnectListAllNetworks(connect *, network ***, unsigned int)
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
    return make_cstring("test");
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
    return make_cstring("");
}

int virDomainSetMetadata(domain *, int, const char *, const char *,
                         const char *, unsigned int)
{
    return 0;
}

char *virDomainGetXMLDesc(domain *, int)
{
    return make_cstring("");
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

char *virNetworkGetXMLDesc(webvirt::network *, unsigned int)
{
    return make_cstring("");
}

int virNetworkFree(network *ptr)
{
    delete ptr;
    return 0;
}

void virConnSetErrorFunc(webvirt::connect *, void *, webvirt::error_function)
{
}
