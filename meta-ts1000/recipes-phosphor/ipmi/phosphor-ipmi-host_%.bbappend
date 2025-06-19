FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " dynamic-sensors"


SRC_URI:append = " file://0001-TS1000-add-0x30-0x55-oem.patch"
