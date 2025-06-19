#!/bin/bash


function set_ip_addr()
{
	busctl call --system  xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.IP.Create IP  ssys "xyz.openbmc_project.Network.IP.Protocol.IPv4" "$1"  $2  "$3"
}

function TS1000_init_fru_ip()
{
	local MAX_RETRIES=3
	local count=0
	local FRU_FILE="/usr/sbin/TS1000FRU*.bin"
	local EEPROM_DEVICE="/sys/bus/i2c/devices/3-0050/eeprom"
	# the first item
	f=$(ls $FRU_FILE 2>/dev/null | head -n1)
	if [[ -z "$f" ]]; then
		echo "No fru file!"
	else
		# write eeprom
		gpioset 0 59=0
		while (( count < MAX_RETRIES )); do
			(( count++ ))
			echo " try ${count}/${MAX_RETRIES} cmd: dd"
			if dd if=$f of=$EEPROM_DEVICE; then 
				echo "success in ${count} time."
				rm $f
				systemctl restart xyz.openbmc_project.FruDevice.service
				sleep 1
				rm /var/configuration/system.json
				systemctl restart  xyz.openbmc_project.EntityManager.service
				break
			else
				echo "failede in ${count} time."
				if (( count < MAX_RETRIES )); then
					sleep 1
				fi
			fi
		done
		gpioset 0 59=1
		# parse ip addr		
		name=${f##*/}
		base_name="${name%.bin}"
		echo "base_name = $base_name"
		IFS='-' read -r prefix ip mask gw <<< "$base_name"  
		echo "IP = $ip"
		echo "MASK = $mask"
		echo "Gateway = $gw"
		# set_ip_addr $ip $mask $gw
	fi
}
