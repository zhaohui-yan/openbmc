FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Potentially-connect-udc-device-incorrectly.patch "
SRC_URI:append = " file://0002-Add-control-for-video-subsampling.patch "
SRC_URI:append = " file://0003-Add-control-for-aspeed-format.patch "
SRC_URI:append = " file://0004-Avoid-frame-drop.patch "

# Use latest revision
SRCREV = "3b201f6961e3de80440384cb4f63822425bca9ec"
