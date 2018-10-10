#!/bin/bash

awk '{
	if($1=="MemTotal:"){
		total=$2/1024
	}
	else if($1=="MemFree:"){
		free=$2/1024
	}
	else if($1=="Buffers:"){
		buffer = $2/1024
	}
	else if($1=="Cached:"){
		cached=$2/1024
	}

}
END{
	used = total-free-cached-buffer;
	ratio = used/total*100;
	printf "mem_total=%0.1f,mem_used=%0.1f,mem_free=%0.1f,cached=%0.1f,buffer=%0.1f,mem_ratio=%0.1f",total,used,free,buffer,cached,ratio
	
}' /proc/meminfo
	
