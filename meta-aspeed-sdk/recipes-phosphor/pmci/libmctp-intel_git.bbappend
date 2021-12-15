FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Add-to-install-libmctp-log-h.patch \
    file://0002-Costomize-buffer-size.patch \
    file://0003-i2c-change-max-payload-size-to-255.patch \
"
