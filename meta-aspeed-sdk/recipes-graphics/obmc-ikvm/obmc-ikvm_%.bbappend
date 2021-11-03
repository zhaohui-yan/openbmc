FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Potentially-connect-udc-device-incorrectly.patch "
SRC_URI:append = " file://0002-Fix-kvm-show-disconnected-if-host-successively-chang.patch "
