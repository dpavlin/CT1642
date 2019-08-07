#!/bin/sh -e

# kill me and all children on ctrl+c
trap "trap - TERM && kill -- -$$" INT TERM EXIT

./fp &
sleep 1
pid=$!
echo "fp pid: $pid"

seq 1 9999 | while read nr ; do
	echo $nr
	echo "number=$nr" > lcd.cfg
	kill -USR1 $pid
	sleep 1
done
