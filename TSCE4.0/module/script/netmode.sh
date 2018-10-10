#!/bin/bash
# get ib mode
#Speed: 1000Mb/s
function read() {
    ethx="eth0";
    net_stat=`ethtool $ethx | grep Speed`;
    echo $net_stat | sed s/[[:space:]]//g | awk '{print substr($0,0,(length-4))}';
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
