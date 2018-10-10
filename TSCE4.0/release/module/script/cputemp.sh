#!/bin/bash
# get cpu temp
#temp:30

function read() {
#	sensors | grep Physical | awk '{print "cpu_temp="substr($4,2,4)}'
	#sensors | grep Physical | awk '{print "cpu_temp="substr($4,2,length($4)-3)}'
    which sensors > /dev/null 2&>1
    if [ $? == 0 ]; then
         #echo "exist"
         sensors | grep Physical | awk '
         BEGIN{temp=""+0;}
         {
             if($4 != "N/A"){

             #cputemp = substr($4,2,length($4)-3);
             cputemp = substr($4,2,4);
             #print $4;
             
             if(cputemp>temp){
                temp = cputemp;
             
             }
             }else{
                 temp="";
            }
         }
         END{
            printf("cpu_temp=%s",temp);
         }'
    else
         echo "cpu_temp="
    fi

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
