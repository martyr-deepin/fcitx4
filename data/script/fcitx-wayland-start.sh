#!/bin/bash

# 脚本解释器 强制设置为 bash
if [ "$BASH" != "/bin/bash" ] && [ "$BASH" != "/usr/bin/bash" ]; then
    bash "$0" "$@"

var1=`systemctl status sshd.service`
var2="active (running)"
result=$(echo $var1 | grep "${var2}")
if [[ "$result" != "" ]]
then
        sleep 1
        fcitx
else
        sleep 2
        bash /bin/fcitx-autostart &
fi
    exit $?
fi