#! /bin/sh

echo load=`uptime  |awk -F" +" '{print $NF}'`
exit 100




