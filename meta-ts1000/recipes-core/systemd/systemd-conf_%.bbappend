FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

CONFIG_FILE_NAME="00-bmc-eth0.network"

SRC_URI:append = " \
    file://${CONFIG_FILE_NAME} \
"

do_install:append() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/${CONFIG_FILE_NAME} ${D}${sysconfdir}/systemd/network/00-bmc-eth0.network
}

FILES:${PN}:append = " \
    ${sysconfdir}/systemd/network/00-bmc-eth0.network \
"
