#!/bin/sh

systemctl --user import-environment GTK_IM_MODULE QT_IM_MODULE XMODIFIERS

if command -v dbus-update-activation-environment >/dev/null 2>&1; then
    dbus-update-activation-environment GTK_IM_MODULE QT_IM_MODULE XMODIFIERS
fi
