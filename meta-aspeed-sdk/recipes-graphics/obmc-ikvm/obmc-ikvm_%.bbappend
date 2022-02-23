FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Potentially-connect-udc-device-incorrectly.patch "
SRC_URI:append = " file://0002-Add-control-for-aspeed-format.patch "
SRC_URI:append = " file://0003-Avoid-frame-drop.patch "

# Use latest revision
SRCREV = "a4f63b38f1e72a3c34c54e275803d945b949483b"
