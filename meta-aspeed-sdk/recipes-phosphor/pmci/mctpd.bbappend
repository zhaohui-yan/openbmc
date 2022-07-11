FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://mctp_pcie_busowner.json \
    file://mctp_pcie_endpoint.json \
"

do_install:append() {
     install -d ${D}/usr/share/mctp
     install -m 0664 ${WORKDIR}/*.json ${D}/usr/share/mctp
}

FILES:${PN} += "/usr/share/mctp/mctp_pcie_busowner.json"
FILES:${PN} += "/usr/share/mctp/mctp_pcie_endpoint.json"
