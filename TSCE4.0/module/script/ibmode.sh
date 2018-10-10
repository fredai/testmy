#!/bin/bash
# get ib mode
#Rate:56
function read() {
    ib_stat=`ibstat | grep Rate`;
    echo $ib_stat | sed s/[[:space:]]//g;
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
