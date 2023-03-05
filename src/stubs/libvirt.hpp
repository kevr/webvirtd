/*
 * Stubs of real libvirt library functions and values.
 *
 * This source should be compiled into test binaries to replace the
 * libvirt library.
 *
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

#include <libvirt_types.hpp>

enum virDomainEventType : int {
    VIR_DOMAIN_EVENT_DEFINED,
    VIR_DOMAIN_EVENT_UNDEFINED,
    VIR_DOMAIN_EVENT_STARTED,
    VIR_DOMAIN_EVENT_SUSPENDED,
    VIR_DOMAIN_EVENT_RESUMED,
    VIR_DOMAIN_EVENT_STOPPED,
    VIR_DOMAIN_EVENT_SHUTDOWN,
    VIR_DOMAIN_EVENT_PMSUSPENDED,
    VIR_DOMAIN_EVENT_CRASHED,
};

enum virDomainEventID : int {
    VIR_DOMAIN_EVENT_ID_LIFECYCLE,
    VIR_DOMAIN_EVENT_ID_REBOOT,
    VIR_DOMAIN_EVENT_ID_RTC_CHANGE,
    VIR_DOMAIN_EVENT_ID_WATCHDOG,
    VIR_DOMAIN_EVENT_ID_IO_ERROR,
    VIR_DOMAIN_EVENT_ID_GRAPHICS,
    VIR_DOMAIN_EVENT_ID_IO_ERROR_REASON,
    VIR_DOMAIN_EVENT_ID_CONTROL_ERROR,
    VIR_DOMAIN_EVENT_ID_BLOCK_JOB,
    VIR_DOMAIN_EVENT_ID_DISK_CHANGE,
    VIR_DOMAIN_EVENT_ID_TRAY_CHANGE,
    VIR_DOMAIN_EVENT_ID_PMWAKEUP,
    VIR_DOMAIN_EVENT_ID_PMSUSPEND,
    VIR_DOMAIN_EVENT_ID_BALLOON_CHANGE,
    VIR_DOMAIN_EVENT_ID_PMSUSPEND_DISK,
    VIR_DOMAIN_EVENT_ID_DEVICE_REMOVED,
    VIR_DOMAIN_EVENT_ID_BLOCK_JOB_2,
    VIR_DOMAIN_EVENT_ID_TUNABLE,
    VIR_DOMAIN_EVENT_ID_AGENT_LIFECYCLE,
    VIR_DOMAIN_EVENT_ID_DEVICE_ADDED,
    VIR_DOMAIN_EVENT_ID_MIGRATION_ITERATION,
    VIR_DOMAIN_EVENT_ID_JOB_COMPLETED,
    VIR_DOMAIN_EVENT_ID_DEVICE_REMOVAL_FAILED,
    VIR_DOMAIN_EVENT_ID_METADATA_CHANGE,
    VIR_DOMAIN_EVENT_ID_BLOCK_THRESHOLD,
    VIR_DOMAIN_EVENT_ID_MEMORY_FAILURE,
    VIR_DOMAIN_EVENT_ID_MEMORY_DEVICE_SIZE_CHANGE,
};

#define VIR_DOMAIN_EVENT_CALLBACK(callback)                                   \
    reinterpret_cast<void (*)(                                                \
        webvirt::connect *, webvirt::domain *, void *)>(callback)

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
int virConnectRegisterCloseCallback(webvirt::connect *,
                                    void (*)(webvirt::connect *, int, void *),
                                    void *, void (*)(void *));
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
int virDomainRef(webvirt::domain *);
int virConnectDomainEventRegisterAny(webvirt::connect *, webvirt::domain *,
                                     int,
                                     void (*)(webvirt::connect *,
                                              webvirt::domain *, void *),
                                     void *, void (*)(void *));

int virConnectDomainEventDeregisterAny(webvirt::connect *, int);
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

// virEvent
int virEventRegisterDefaultImpl();
int virEventAddTimeout(int, void (*)(int, void *), void *, void (*)(void *));
int virEventRunDefaultImpl();

void virConnSetErrorFunc(webvirt::connect *, void *, webvirt::error_function);

#endif /* STUBS_LIBVIRT_HPP */
