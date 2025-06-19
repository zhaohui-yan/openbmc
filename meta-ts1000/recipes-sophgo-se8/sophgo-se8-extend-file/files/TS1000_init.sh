#!/bin/bash

source /usr/sbin/init_eeprom_mac.sh
source /usr/sbin/init_fru_ip.sh
source /usr/sbin/init_hostsnmp.sh

echo "TS1000_init_hostname"
TS1000_init_hostname

echo "TS1000_init_fru_ip"
TS1000_init_fru_ip
sleep 1
echo "TS1000_init_eeprom_mac"
TS1000_init_eeprom_mac
# sleep 5
