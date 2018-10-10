#! /bin/sh

killall collectord >/dev/null 2>&1

ipcrm -M 0x0fabcdef >/dev/null 2>&1
ipcrm -S 0x0fabcdef >/dev/null 2>&1

