webvirtd
--------

![lint](https://github.com/kevr/webvirtd/actions/workflows/lint.yaml/badge.svg?branch=master)
![build](https://github.com/kevr/webvirtd/actions/workflows/build.yaml/badge.svg?branch=master)
![tests](https://github.com/kevr/webvirtd/actions/workflows/tests.yaml/badge.svg?branch=master)
[![license](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

System libvirt control daemon for the webvirt project

Configuration
-------------

To compile webvirtd, it must first be configured using [meson](https://mesonbuild.com/):

    ## Release build
    $ meson setup --buildtype release -Ddisable_tests=true builddir

    ## Development build
    $ meson setup --buildtype debug -Db_coverage=true builddir

Compilation
-----------

After [configuration](#configuration), users can build the project using
[ninja](https://ninja-build.org/manual.html) or
[samu](https://github.com/michaelforney/samurai):

    $ (ninja|samu) -C builddir

Installation
------------

To install the project, run the *install* target after [compilation](#compilation):

    $ meson -C builddir install --skip-subprojects

Note: *--skip-subprojects* is recommended here, as subprojects are built statically and
do not need to be housed within the root filesystem. In addition, these subproject
installations can conflict with Linux distribution packages which install the same
projects.

What the meson install target does:

1. Installs systemd services to `{prefix}/lib/systemd/system`
2. Creates the `webvirt` user utilized by the systemd service

Before continuing, you should [configure libvirt user access](doc/libvirt.md).

Running
-------

The webvirtd binary contacts libvirtd using the `qemu+ssh` protocol. Therefore, libvirtd
access depends on authentication provided by SSH.

See [libvirt user access](doc/libvirt.md) for details on configuring libvirtd access control.

#### Development

In development, webvirtd can be run directly by any user:

    $ ./builddir/src/webvirtd --help

Users should ensure that `--socket` used with webvirtd and `WEBVIRTD_SOCKET`
used with [webvirt_api](https://github.com/kevr/webvirt_api) are matched,
and that [webvirt_api](https://github.com/kevr/webvirt_api) has permission to
converse with webvirtd's socket.

#### Production

In production, webvirtd should be run using the systemd services provided by install:

    ## Enable at boot and start the service
    # systemctl enable --now webvirtd.service

The unix socket deployed by [webvirtd.service](res/webvirtd.service) resides at
`/var/run/webvirtd/webvirtd.sock`. To access the unix socket, the accessing user
must be in the `webvirt` group:

    # gpasswd -a some_user webvirt
    # sudo -u some_user curl \
        --unix-socket /var/run/webvirtd/webvirtd.sock \
        http://localhost/users/test/domains/

API Documentation
-----------------

Read the API documentation at SwaggerHub: [https://app.swaggerhub.com/apis-docs/kevr/webvirtd](https://app.swaggerhub.com/apis-docs/kevr/webvirtd)

Licensing
---------

This project operates under the [Apache 2.0 LICENSE](LICENSE) along with
an [Apache 2.0 NOTICE](NOTICE) which contains attributions of work.
