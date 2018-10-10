#!/bin/bash
# get physics disk used size
#maxused:3,mounted:/home 
AWK=/bin/awk
function read() {
    info="df -h"
    #info="dmidecode"
    eval ${info} | awk -F ' ' '
BEGIN{
temp=""+0;
mountedon="";}
{
         #split($0,files," ");
         for(i=1;i<=NF;i++){
             #print($i);
             if($i ~ /^[0-9%]+$/){
 		 if($i!=0){
                    values=substr($i,0,length($i)-1)+0;
                    if(values>temp){
                        temp=values+0;
                        mountedon=$(i+1)+"";
                        if($(i+1)=="/"){
                            mountedon="/";
                        }else{   
                            mountedon=$(i+1);
                        }
                    }
                 }
             }
         
         }
    }
   END{
    printf("disk_maxused:%d,mounted:%s\n",temp,mountedon);
}'; 
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
