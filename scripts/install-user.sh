#!/bin/bash
user=webvirt
group=webvirt

error=
if ! grep -q $user /etc/passwd; then
  if [ $UID -eq 0 ]; then
    useradd $user
  else
    echo "Services installed rely on the '$user' user, but it could not be found." 1>&2
    echo "Run \`useradd $user\` as root." 1>&2
    echo 1>&2
    error=1
  fi
fi

if ! grep -q $group /etc/group; then
  if [ $UID -eq 0 ]; then
    groupadd $group
  else
    echo "Services installed rely on the '$group' group, but it could not be found." 1>&2
    echo "Run \`groupadd $group\` as root." 1>&2
    echo 1>&2
    error=1
  fi
fi

if [ "$error" ]; then
  exit $error
fi
