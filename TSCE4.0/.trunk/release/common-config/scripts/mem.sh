#! /bin/sh
echo mem_used=`free -m | grep Mem |awk -F" +" '{print $3}'`
exit 100




