SUMMARY = "OpenBMC for AspeedTech - System"

PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"

PACKAGES = " \
        ${PN}-chassis \
        ${PN}-extras \
        ${PN}-fans \
        ${PN}-flash \
        ${PN}-system \
	"

PROVIDES_append = " virtual/obmc-chassis-mgmt"
PROVIDES_append = " virtual/obmc-fan-mgmt"
PROVIDES_append = " virtual/obmc-flash-mgmt"
PROVIDES_append = " virtual/obmc-system-mgmt"

RPROVIDES_${PN}-chassis = " virtual-obmc-chassis-mgmt"
RPROVIDES_${PN}-fans = " virtual-obmc-fan-mgmt"
RPROVIDES_${PN}-flash = " virtual-obmc-flash-mgmt"
RPROVIDES_${PN}-system = " virtual-obmc-system-mgmt"

SUMMARY_${PN}-chassis = "AspeedTech Chassis"
RDEPENDS_${PN}-chassis = " \
        x86-power-control \
        "

SUMMARY_${PN}-fans = "AspeedTech Fans"
RDEPENDS_${PN}-fans = " \
        phosphor-pid-control \
        "

SUMMARY_${PN}-flash = "AspeedTech Flash"
RDEPENDS_${PN}-flash = " \
        phosphor-software-manager \
        "

SUMMARY_${PN}-system = "AspeedTech System"
RDEPENDS_${PN}-system = " \
        phosphor-ipmi-ipmb \
        phosphor-hostlogger \
        phosphor-sel-logger \
        ipmitool \
        phosphor-post-code-manager \
        phosphor-host-postd \
        "
