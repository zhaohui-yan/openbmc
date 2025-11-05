FILESPATH := "${THISDIR}/files/:"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""


# FRU_FILE_NAME="TS1000FRU-TS10002406170005.bin"

SRC_URI += " \
           file://se8PowerControl.sh \
           file://snmp_shell.sh \
           file://snmpd.conf \
           file://TS1000_init.service \
           file://localtime \
           file://init_hostsnmp.sh \
           file://init_fru_ip.sh \
           file://init_eeprom_mac.sh \
           file://TS1000_init.sh \
           file://smbios2 \
           file://se8systeminfo \
           file://snmp_manager.sh \
           "

# SRC_URI += "file://${FRU_FILE_NAME}"

S = "${WORKDIR}"
RDEPENDS:${PN} += "bash"

inherit  systemd bash-completion obmc-phosphor-systemd

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} = " \
    TS1000_init.service \
"
FILES:${PN}  += "${systemd_system_unitdir}/TS1000_init.service"
FILES:${PN}  += "/var/lib/smbios/smbios2"

do_install () {
  install -d ${D}/${sysconfdir}
  install -m 0644 ${WORKDIR}/localtime ${D}${sysconfdir}/
	install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/se8PowerControl.sh ${D}/${sbindir}
  # install -d ${D}/${sbindir}
  # install -m 0755 ${WORKDIR}/${FRU_FILE_NAME} ${D}/${sbindir}
  install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/snmp_shell.sh ${D}/${sbindir}
  install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/snmp_manager.sh ${D}/${sbindir}
  install -d ${D}${sysconfdir}/snmp
  install -m 644 ${WORKDIR}/snmpd.conf ${D}${sysconfdir}/snmp/
  install -d ${D}${systemd_system_unitdir}
  install -m 0644 ${WORKDIR}/TS1000_init.service ${D}${systemd_system_unitdir}
  install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/init_hostsnmp.sh ${D}/${sbindir}
  install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/init_fru_ip.sh ${D}/${sbindir}
  install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/init_eeprom_mac.sh ${D}/${sbindir}
  install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/TS1000_init.sh ${D}/${sbindir}
  install -d ${D}/var/lib/smbios
  install -m 0644 ${WORKDIR}/smbios2 ${D}/var/lib/smbios/

}
