FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append_ast-abr = " \
    file://0001-mirror-boot-for-static.patch \
    file://0002-obmc-flash-bmc-for-abr.patch \
"

SYSTEMD_SERVICE_${PN}-updater_append_ast-abr = " \
    obmc-flash-bmc-mirroruboot.service \
"
