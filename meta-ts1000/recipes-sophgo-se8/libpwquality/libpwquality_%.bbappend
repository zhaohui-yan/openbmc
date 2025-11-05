
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += " \
        file://pwquality.conf \
        "

do_install:append() {
    install -d ${D}/etc/security
    install -m 0644 ${WORKDIR}/pwquality.conf ${D}/etc/security
}
