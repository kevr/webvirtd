webvirtd
--------

![lint](https://github.com/kevr/webvirtd/actions/workflows/lint.yaml/badge.svg?branch=master)
![build](https://github.com/kevr/webvirtd/actions/workflows/build.yaml/badge.svg?branch=master)
![tests](https://github.com/kevr/webvirtd/actions/workflows/tests.yaml/badge.svg?branch=master)
[![license](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

System libvirt control daemon for the webvirt project

Installation
------------

`webvirtd` accesses libvirt using the `qemu+ssh` protocol. To give
webvirtd access to a particular user, the webvirt user's public key
must allow access to that user via SSH.

In this example, we will use the `webvirt` user:

    # useradd -m webvirt
    # sudo -u webvirt ssh-keygen
    # install -m644 /home/webvirt/.ssh/id_rsa.pub /etc/ssh/authorized_keys

To allow access to all users on the system, `/etc/ssh/sshd_config` can
contain a custom AuthorizedKeysFile option:

    ...
    AuthorizedKeysFile /etc/ssh/authorized_keys .ssh/authorized_keys
    ...

Running
-------

`webvirtd` should be run as the user who owns the key used to
authenticate users locally over SSH:

    ## As `webvirt` user
    $ webvirtd --socket=/home/webvirt/webvirtd.sock

Licensing
---------

This project operates under the [Apache 2.0 LICENSE](LICENSE) along with
an [Apache 2.0 NOTICE](NOTICE) which contains attributions of work.
