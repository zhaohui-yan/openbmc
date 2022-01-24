FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Potentially-connect-udc-device-incorrectly.patch "
# Use latest revision
SRCREV = "3b201f6961e3de80440384cb4f63822425bca9ec"
