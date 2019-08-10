#!/bin/sh -xe

echo "test=$1\n" > lcd.cfg
killall -s USR1 fp

