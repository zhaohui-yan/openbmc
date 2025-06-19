#!/bin/bash
function se8_power_on()
{
	gpioset 0 32=0
	echo "0"
	usleep 500000
	gpioset 0 32=1
	echo "1"
	sleep 6
	gpioset 0 32=0
	echo "2"
	sleep 3
	gpioset 0 32=1
	echo "3"
}

function se8_power_off()
{
	gpioset 0 32=0
	usleep 500000
	gpioset 0 32=1
	usleep 500000
	gpioset 0 32=0
}

function se8_power_reset()
{
	gpioset 0 34=0
	usleep 50000
	gpioset 0 34=1
}
