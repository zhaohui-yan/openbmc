FILESPATH := "${THISDIR}/files/:"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""


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
           file://TS1000FRU-10.8.98.131-25-10.8.98.253.bin \
           file://TS1000FRU-10.10.120.211-25-10.10.120.253.bin \
           "

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
  # install -m 0755 ${WORKDIR}/TS1000FRU-10.10.120.211-25-10.10.120.253.bin ${D}/${sbindir}
  # install -d ${D}/${sbindir}
  # install -m 0755 ${WORKDIR}/TS1000FRU-10.8.98.131-25-10.8.98.253.bin ${D}/${sbindir}
  install -d ${D}/${sbindir}
  install -m 0755 ${WORKDIR}/snmp_shell.sh ${D}/${sbindir}
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
