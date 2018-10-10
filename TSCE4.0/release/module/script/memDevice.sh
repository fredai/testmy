#!/bin/bash
# get physics mem count
#count:3,size:20000
#size=0;
#count=0;
function read() {
    info="dmidecode | grep -A 9 \"Memory Device$\""
    #info="dmidecode"
    eval ${info} | awk -F ': ' '{
         if($1=="	Size" ){
             if($2!="No Module Installed"){
                  size=size+$2;
                  ((count++));
             }
        
        }
    
    }
   END{
    printf("mem_installed_num:%s,mem_volume:%s\n",count,size);
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
