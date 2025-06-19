#!/bin/bash



# get eeprom printf info
new_mac=`/usr/bin/sophgo-eeprom-mac -d /sys/bus/i2c/devices/3-0050/eeprom`
value=$?
if [ $value = 1 ] ;then
    echo "get mac addr from eeprom : $new_mac"
    old_mac=`cat /sys/class/net/eth0/address`
    new_mac1=$(echo $new_mac | tr [a-z] [A-Z])
    old_mac1=$(echo $old_mac | tr [a-z] [A-Z])
    if [ "$old_mac1"x = "$new_mac1"x ] ;then
        echo "old_mac eq new_mac"
    else
        busctl set-property xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.MACAddress MACAddress s $new_mac
    fi
else
    echo "ERROR: Can not get valid mac addr from eeprom !"
fi
