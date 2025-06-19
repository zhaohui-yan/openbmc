FILESEXTRAPATHS:append := ":${THISDIR}/${PN}"

SRC_URI += " \
            file://sophgo_se8_board.json \
            "
FILES:${PN}  += "${prefix_native}/share/entity-manager/configurations/sophgo_se8_board.json"

do_install:append() {

    install -d ${D}/${prefix_native}/share/entity-manager/configurations/
    install -m 0644 ${WORKDIR}/sophgo_se8_board.json ${D}/${prefix_native}/share/entity-manager/configurations

    # install -m 0444 ${WORKDIR}/blacklist.json -D -t ${D}/usr/share/entity-manager
}
