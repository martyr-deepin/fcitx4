#!/bin/bash

# 脚本解释器 强制设置为 bash
if [ "$BASH" != "/bin/bash" ] && [ "$BASH" != "/usr/bin/bash" ]; then
    bash "$0" "$@"
    exit $?
fi

var1=`systemctl status dbus.service`
var2="active (running)"
result=$(echo $var1 | grep "${var2}")
while [ "$result" == "" ]
do
    sleep 2
    var1=`systemctl status dbus.service`
    result=$(echo $var1 | grep "${var2}")
done