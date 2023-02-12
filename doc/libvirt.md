Overview
--------

*webvirtd* connects to a libvirtd instance using the *qemu+ssh* protocol.

### libvirt qemu+ssh Review

If we want to access a user's libvirt session over ssh, we could do so
using the following command:

    $ virsh -c 'qemu+ssh://user@localhost'

The aforementioned command will fail without authentication setup
between the virsh user and the target libvirt user.

To rectify this, the user executing virsh submits their public
key to the target user:

    $ ssh-copy-id user@localhost

Once the virsh user's public key is on the target, the command would
succeed.

### How webvirtd uses qemu+ssh

In order to access a user's libvirt session, webvirtd utilizes the ssh
communication provided by libvirt as the user running the webvirtd
executable.

This means that access to libvirt from webvirtd's perspective depends
on the fact that the webvirtd user has valid ssh authentication access
to the target user.

    $ curl --unix-socket webvirtd.sock http://localhost/users/test/domains/

The above command, which contacts the `/users/(user)/domains/` route served
by webvirtd, will deduce the target user based off of the request URI and
produce a *qemu+ssh* libvirt URI used to access libvirt:

    qemu+ssh://test@localhost/session

### Controlling libvirt users

#### Create webvirtd user keys

The user running webvirtd will need an ssh key pair to utilize public key
authentication via ssh toward targetted users.

If the webvirt user is running webvirtd:

    # sudo -u webvirt ssh-keygen

#### Allow all users

To allow libvirt ssh access to all users in the system, `/etc/ssh/sshd_config`
can be modified to rely on a system-wide authorized_keys for access.

    # /etc/ssh/sshd_config
    ...
    AuthorizedKeysFile /etc/ssh/authorized_keys .ssh/authorized_keys
    ...

/etc/ssh/authorized_keys should be a copy of the webvirt user's public ssh key:

    # install -m660 /home/webvirt/.ssh/id_rsa.pub /etc/ssh/authorized_keys

#### Allow specific users

To allow particular users in the system, the webvirtd user's public key should
be put into their ~/.ssh/authorized_keys:

    # cat /home/webvirt/.ssh/id_rsa.pub >> /home/target_user/.ssh/authorized_keys
    # chown target_user:target_user /home/target_user/.ssh/authorized_keys
