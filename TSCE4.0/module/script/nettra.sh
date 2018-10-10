#!/bin/sh

tmpDir="/tmp/tsced/"
tmpCpu=".cpu.tmp"

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


function readcpu()
{
cat "$tmpDir$tmpCpu" | awk '{print $1,$2,$3,$4,$5}' | while read param1 param2 param3 param4 param5
do
time_new=$(date +%s.%N)
param5=$(timeInterval $param5 $time_new)	
cat /proc/net/dev | grep -v "virbr0\|lo\|Receive\|packets\|ib" | tr : " " | awk -v p1=$param1 -v p2=$param2 -v p3=$param3 -v p4=$param4 -v timeinterval=$param5 -v nextime=$time_new '
BEGIN{rece="";
      send="";
      tsend="";
      trece="";
      psend="";
      prece="";
      psendtemp="";
      precetemp="";
}
{
trece=trece+$2;
tsend=tsend+$10;
psend=psend+$11;
prece=prece+$3;
}
END{
	rece = trece-p1;
	send = tsend-p2;
        psendtemp = psend-p4;
        precetemp = prece-p3;
	printf "receive_rate=%.2f,transmit_rate=%.2f,receive_pack=%d,transmit_pack=%d\n", rece/(1024*1024*timeinterval/1000), send/(1024*1024*timeinterval/1000), precetemp/(timeinterval/1000), psendtemp/(timeinterval/1000);
	#printf "ReceivePack=%d,TransmitPack=%d\n", precetemp/(timeinterval/1000), psendtemp/(timeinterval/1000);
	print trece" "tsend" "prece" "psend" "nextime > "/tmp/tsced/.cpu.tmp"
}'


done
}

function start()
{
mkdir -p $tmpDir
touch $tmpDir$tmpCpu

time=$(date +%s.%N)
cat /proc/net/dev | grep -v "virbr0\|lo\|Receive\|packets\|ib" | tr : " " | awk -v time=$time '
BEGIN{rece="";
      send="";
      tsend="";
      psend="";
      prece="";
      trece="";
}
{
trece=trece+$2;
tsend=tsend+$10;
psend=pend+$11;
prece=prece+$3;
}
END{print trece" "tsend" "prece" "psend" "time > "/tmp/tsced/.cpu.tmp"
}'
}


case $1 in
(start)
	start
	;;
(read)
	readcpu
	;;
(*)
	echo "error"
	;;
esac
