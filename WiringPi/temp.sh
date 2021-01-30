#!/bin/sh -e

cd /home/pi/CT1642/WiringPi/

killall fp || true

# kill me and all children on ctrl+c
trap "trap - TERM && kill -- -$$" INT TERM EXIT

echo > lcd.cfg

nice -n -20 ./fp &
sleep 1
pid=$!
echo "fp pid: $pid"

mosquitto_sub -h rpi2 -t 'stat/boiler/DS18B20/temperature' | while read temp ; do
	a=$( echo $temp | cut -d\. -f1 )
	b=$( echo $temp | cut -d\. -f2 )
	echo "time_hour=$a\ntime_min=$b\n" > lcd.cfg.new
	if ! diff lcd.cfg lcd.cfg.new >/dev/null ; then
		mv lcd.cfg.new lcd.cfg
		kill -USR1 $pid
		#echo "$a:$b"
	fi
done
