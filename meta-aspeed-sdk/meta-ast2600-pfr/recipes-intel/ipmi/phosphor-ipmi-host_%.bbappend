FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://phosphor-ipmi-host.service "

RRECOMMENDS:${PN}:remove = "phosphor-settings-manager"
