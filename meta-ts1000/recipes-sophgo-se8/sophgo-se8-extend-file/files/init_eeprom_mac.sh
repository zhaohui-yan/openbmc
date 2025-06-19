#!/bin/bash


function TS1000_init_eeprom_mac()
{
	# get eeprom printf info
	ts1000_new_mac=`/usr/bin/sophgo-eeprom-mac -d /sys/bus/i2c/devices/3-0050/eeprom`
	ts1000_value=$?
	if [ $ts1000_value = 1 ] ;then
			echo "get mac addr from eeprom : $ts1000_new_mac"
			ts1000_old_mac=`cat /sys/class/net/eth0/address`
			ts1000_new_mac1=$(echo $ts1000_new_mac | tr [a-z] [A-Z])
			ts1000_old_mac1=$(echo $ts1000_old_mac | tr [a-z] [A-Z])
			if [ "$ts1000_old_mac1"x = "$ts1000_new_mac1"x ] ;then
					echo "old_mac eq new_mac"
			else
					busctl set-property xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.MACAddress MACAddress s $ts1000_new_mac
			fi
	else
			echo "ERROR: Can not get valid mac addr from eeprom !"
	fi
}
