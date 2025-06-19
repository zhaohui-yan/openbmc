SUMMARY = "get eeprom mac addr"
DESCRIPTION = "get eeprom mac addr"

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

FILESPATH := "${THISDIR}/files:"



SRC_URI += " \
    file://Makefile \
    file://eeprom-mac.c \
    file://eeprom-mac.sh \
    file://sophgo-eeprom-mac.service \
"



S = "${WORKDIR}"

do_compile () {
    pwd
    make
}

RDEPENDS:${PN} += "bash"

inherit  systemd bash-completion obmc-phosphor-systemd

# SYSTEMD_AUTO_ENABLE = "enable"
# SYSTEMD_SERVICE_${PN} = "sophgo-eeprom-mac.service"
# FILES:${PN}  += "${systemd_system_unitdir}/sophgo-eeprom-mac.service"


TARGET_CC_ARCH += "${LDFLAGS}"
do_install() {
    install -d ${D}${bindir}
    install -m 0755 sophgo-eeprom-mac ${D}${bindir}
    # install -d ${D}/${sbindir}
    # install -m 0755 ${WORKDIR}/eeprom-mac.sh ${D}/${sbindir}
    # install -d ${D}${systemd_system_unitdir}
    # install -m 0644 ${WORKDIR}/sophgo-eeprom-mac.service ${D}${systemd_system_unitdir}
}
