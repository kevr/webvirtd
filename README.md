webvirtd
--------

![lint](https://github.com/kevr/webvirtd/actions/workflows/lint.yaml/badge.svg?branch=master)
![build](https://github.com/kevr/webvirtd/actions/workflows/build.yaml/badge.svg?branch=master)
![tests](https://github.com/kevr/webvirtd/actions/workflows/tests.yaml/badge.svg?branch=master)
[![license](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

System libvirt control daemon for the webvirt project

Installation
------------

To install the project, configure and run install:

    $ meson setup --buildtype release -Ddisable_tests=true release
    $ cd release

    ## Build the project
    release$ ninja

    ## Install it without subprojects
    release$ sudo meson install --skip-subprojects

The meson install target:

1. Installs systemd services to `{prefix}/lib/systemd/system`
2. Creates the `webvirt` user utilized by the systemd service

Before continuing, you should [configure libvirt user access](doc/libvirt.md).

Running
-------

webvirtd should be run using the systemd services provided by install:

    ## Enable at boot
    # systemctl enable webvirtd.service

    ## Start the service
    # systemctl start webvirtd.service

The unix socket deployed by webvirtd.service resides at
`/var/run/webvirtd/webvirtd.sock`. To access the unix socket as other
users, add them to the `webvirt` group:

    # gpasswd -a some_user webvirt
    # sudo -u some_user curl \
        --unix-socket /var/run/webvirtd/webvirtd.sock \
        http://localhost/users/test/domains/

API Documentation
-----------------

Read the API documentation at SwaggerHub: [https://app.swaggerhub.com/apis/kevr/webvirtd](https://app.swaggerhub.com/apis/kevr/webvirtd)

Licensing
---------

This project operates under the [Apache 2.0 LICENSE](LICENSE) along with
an [Apache 2.0 NOTICE](NOTICE) which contains attributions of work.
