FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Add-to-install-libmctp-log.h.patch \
    file://0002-disable-all-warnings-being-treated-as-errors.patch \
    file://0003-Add-function-to-change-the-file-name-of-mctp-device.patch \
"
