FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append:ast2600-pfr = " file://0001-change-boot-addr.patch "
SRC_URI:append:ast2600-dcscm = " file://0001-change-boot-addr-dcscm.patch "

