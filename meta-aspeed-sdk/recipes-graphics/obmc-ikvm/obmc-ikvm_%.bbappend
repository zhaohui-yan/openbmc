FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Potentially-connect-udc-device-incorrectly.patch "

# Use latest revision
SRCREV = "ee09e3033a453565034b2b9bf4f2e0cbc8323ccd"
