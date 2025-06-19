#!/bin/bash

function TS1000_init_hostname()
{
	local HOSTNAME=$(/bin/hostname)
	if [ -f /etc/hostname ];then
		HOSTNAME=$(cat /etc/hostname)
	fi
	# busctl set-property xyz.openbmc_project.Network \
	# 										/xyz/openbmc_project/network/config \
	# 										xyz.openbmc_project.Network.SystemConfiguration \
	# 										HostName \
	# 										s \
	# 										$HOSTNAME

	hostnamectl set-hostname TS1000
}
	
