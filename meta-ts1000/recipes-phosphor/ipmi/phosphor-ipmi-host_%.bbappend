FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# DEPENDS += "se8-system-info"

PACKAGECONFIG:append = " dynamic-sensors"


SRC_URI:append = " \
					file://0001-TS1000-add-0x30-0x55-oem.patch \
					file://0002-Store-se8SystemInfo-to-dbus.patch \
"
