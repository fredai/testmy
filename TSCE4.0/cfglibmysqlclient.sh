#!/bin/bash

version=`uname -r`
uname=`uname -m`

lib64_path="/usr/lib64/mysql/"
lib32_path="/usr/lib32/mysql/"
mysql_so="libmysqlclient.so"
el7_so="libmysqlclient.so.18"
el6_so="libmysqlclient.so.16"
profile="/etc/profile"


if [ $uname == "x86_64" ]
then

	mysqlDir=$lib64_path
	if [ ! -d "$mysqlDir" ]
	then
		echo -e '\033[0;31;1mmysql forder not exist \033[0m'
		exit
	fi

	
	if [[ $version =~ "el7" ]]
	then
		file_name=$lib64_path$el7_so
		if [ ! -f "$file_name" ]
		then
			`ln -s $lib64_path$mysql_so $lib64_path$el7_so`
			echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$lib64_path" >> $profile
			source $profile
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
		else 
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
			exit
		fi

	fi

	if [[ $version =~ "el6" ]]
	then
		file_name=$lib64_path$el6_so
		if [ ! -f "$file_name" ]
		then
			`ln -s $lib64_path$mysql_so $lib64_path$el6_so`
			echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$lib64_path" >> $profile
			source $profile
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
		else 
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
			exit
		fi
	fi

fi

if [ $uname == "i386" ]
then

	mysqlDir=$lib32_path
	if [ ! -d "$mysqlDir" ]
	then
		echo -e '\033[0;31;1mmysql forder not exist \033[0m'
		exit
	fi

	
	if [[ $version =~ "el7" ]]
	then
		file_name=$lib32_path$el7_so
		if [ ! -f "$file_name" ]
		then
			`ln -s $lib32_path$mysql_so $lib32_path$el7_so`
			echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$lib32_path" >> $profile
			source $profile
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
		else 
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
			exit
		fi

	fi

	if [[ $version =~ "el6" ]]
	then
		file_name=$lib32_path$el6_so
		if [ ! -f "$file_name" ]
		then
			`ln -s $lib32_path$mysql_so $lib32_path$el6_so`
			echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$lib32_path" >> $profile
			source $profile
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
		else 
			echo -e '\033[0;31;1mlibmysqlclient config success \033[0m'
			exit
		fi
	fi

fi


