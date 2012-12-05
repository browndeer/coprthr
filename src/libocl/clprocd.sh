#!/usr/local/bin/bash

while :
do

pids=`ls /var/clproc`

echo $pids

for p in $pids
do

ps -p $p > /dev/null 2>&1

if [ $? -ne 0 ]; then
	echo "process " $p " is crashed";
else
	echo "process " $p " is running";
fi

if [ $(find /var/clproc/$p -amin +5 -type f -name status ) ];then
	echo "$p is old"
	unlink /var/clproc/$p/status
	rmdir /var/clproc/$p
else
	echo "$p is new"
fi

done

sleep  2
echo tic

done

