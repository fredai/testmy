#!/bin/bash
# get system load
#one:0.3,five:0.8,fifteen:1.2

function read() {
    #uptime | cut -d "," -f4,5,6 | awk '{split($3,one,",");split($4,five,",");split($5,fifteen,",");print "load_one="one[1]",load_five="five[1]",load_fifteen="fifteen[1]}'
    uptime | awk -F 'load average: ' '{print $2}' | awk '{print "load_one="$1"load_five="$2"load_fifteen="$3}'
}

function start() {
   echo "ok"
}

case $1 in
(start)
	start
	;;
(read)
	read
	;;
(*)
	echo "error"
	;;
esac
