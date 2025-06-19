PROVIDES:remove = "virtual/obmc-host-ipmi-hw"
RPROVIDES:${PN}:remove = "virtual-obmc-host-ipmi-hw"
RRECOMMENDS:${PN}:remove = "phosphor-ipmi-host"
SYSTEMD_SERVICE:${PN}:remove = "ssifbridge.service"
do_install:append() {
    rm -f ${D}${systemd_system_unitdir}/ssifbridge.service
    rmdir --ignore-fail-on-non-empty ${D}${systemd_system_unitdir} || true
    rmdir --ignore-fail-on-non-empty ${D}/usr/lib/systemd || true
    rmdir --ignore-fail-on-non-empty ${D}/usr/lib || true
}

FILES:${PN} = "${bindir}/* ${libdir}/*"
