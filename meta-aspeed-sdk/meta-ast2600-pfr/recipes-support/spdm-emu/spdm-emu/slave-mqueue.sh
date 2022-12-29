#!/bin/sh

BUS=${1:-4}
SA=${2:-10}
MQ=/sys/bus/i2c/devices/${BUS}-00${SA}

if [ ! -d "$MQ" ]; then
	echo slave-mqueue 0x${SA} > /sys/bus/i2c/devices/i2c-${BUS}/new_device
	sleep 1
else
	hexdump -C ${MQ}/slave-mqueue
fi
