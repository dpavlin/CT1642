#!/bin/sh -xe

echo "delay=$1\n" > lcd.cfg
killall -s USR1 fp

