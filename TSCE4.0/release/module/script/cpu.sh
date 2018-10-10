#!/bin/bash
config="/tmp/tsce4.0.tmp/.cpu_tmp"

function cpuinfo(){
	cat /proc/stat | grep "^cpu " | awk '
	{
		total=$2+$3+$4+$5+$6+$7+$8+$9+$10+$11
		print total" "$5" "$2" "$4" "$6
	}'
}

cpu1=( $(cpuinfo) )
sleep 0.2
cpu2=( $(cpuinfo) )

total=$(( ${cpu2[0]}-${cpu1[0]} ))
free=$(( ${cpu2[1]}-${cpu1[1]} ))
usr=$(( ${cpu2[2]}-${cpu1[2]} ))
sys=$(( ${cpu2[3]}-${cpu1[3]} ))
io=$(( ${cpu2[4]}-${cpu1[4]} ))

echo "$total $free $usr $sys $io" | awk '{
	free=100*($2/$1)
	usr=100*($3/$1)
	sys=100*($4/$1)
	io=100*($5/$1)
#	printf "free=%.2f\n,usr=%.2f\n,sys=%.2f\n,io=%.2f\n",free,usr,sys,io
	printf "cpu_user=%.2f,cpu_sys=%.2f,cpu_iowait=%.2f,cpu_idle=%.2f",free,usr,sys,io
}'

function timeInterval() {  
    start=$1  
    end=$2  
     
    start_s=$(echo $start | cut -d '.' -f 1)  
    start_ns=$(echo $start | cut -d '.' -f 2)  
    end_s=$(echo $end | cut -d '.' -f 1)  
    end_ns=$(echo $end | cut -d '.' -f 2)  
  
	time=$(( ( 10#$end_s - 10#$start_s ) * 1000 + ( 10#$end_ns / 1000000 - 10#$start_ns / 1000000 )  ))
	echo $time
}  

function read() {
	
	###################get new and old time and sub######################
	time_old=`cat $config | cut -d ':' -f 1`			#read old time  #
	time_new=$(date +%s.%N)								#get new time   #
	time=$(timeInterval $time_old $time_new)			#sub			#
	#####################################################################
}

function start() {
	`touch  $config`
	time_new=$(date +%s.%N)
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
