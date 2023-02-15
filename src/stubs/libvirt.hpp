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
#ifndef STUBS_LIBVIRT_HPP
#define STUBS_LIBVIRT_HPP

#include "../libvirt_types.hpp"

enum virDomainState : int {
    VIR_DOMAIN_NOSTATE,
    VIR_DOMAIN_RUNNING,
    VIR_DOMAIN_BLOCKED,
    VIR_DOMAIN_PAUSED,
    VIR_DOMAIN_SHUTDOWN,
    VIR_DOMAIN_SHUTOFF,
    VIR_DOMAIN_CRASHED,
    VIR_DOMAIN_PMSUSPENDED,
};

enum metadataType : int {
    VIR_DOMAIN_METADATA_TITLE,
    VIR_DOMAIN_METADATA_DESCRIPTION,
    VIR_DOMAIN_METADATA_ELEMENT,
    VIR_DOMAIN_METADATA_LAST,
};

// virConnect
webvirt::connect *virConnectOpen(const char *);

char *virConnectGetCapabilities(webvirt::connect *);
char *virConnectGetHostname(webvirt::connect *);
int virConnectGetLibVersion(webvirt::connect *, unsigned long *);
int virConnectGetMaxVcpus(webvirt::connect *, const char *);
char *virConnectGetSysinfo(webvirt::connect *, unsigned int);
const char *virConnectGetType(webvirt::connect *);
char *virConnectGetURI(webvirt::connect *);
int virConnectGetVersion(webvirt::connect *, unsigned long *);
int virConnectIsEncrypted(webvirt::connect *);
int virConnectIsSecure(webvirt::connect *);

int virConnectListAllDomains(webvirt::connect *, webvirt::domain ***, int);
int virConnectListAllNetworks(webvirt::connect *, webvirt::network ***,
                              unsigned int);
int virConnectClose(webvirt::connect *);

// virDomain
webvirt::domain *virDomainLookupByName(webvirt::connect *, const char *);
int virDomainCreate(webvirt::domain *);
int virDomainGetState(webvirt::domain *, int *, int *, int);
int virDomainGetID(webvirt::domain *);
char *virDomainGetName(webvirt::domain *);
int virDomainGetAutostart(webvirt::domain *, int *);
int virDomainSetAutostart(webvirt::domain *, int);
char *virDomainGetMetadata(webvirt::domain *, int, const char *, unsigned int);
int virDomainSetMetadata(webvirt::domain *, int, const char *, const char *,
                         const char *, unsigned int);
char *virDomainGetXMLDesc(webvirt::domain *, int);
webvirt::domain *virDomainDefineXML(webvirt::connect *, const char *);
int virDomainGetBlockInfo(webvirt::domain *, const char *,
                          webvirt::block_info *, int);
int virDomainShutdown(webvirt::domain *);
int virDomainFree(webvirt::domain *);

// virNetwork
char *virNetworkGetXMLDesc(webvirt::network *, unsigned int);
int virNetworkFree(webvirt::network *);

void virConnSetErrorFunc(webvirt::connect *, void *, webvirt::error_function);

#endif /* STUBS_LIBVIRT_HPP */
