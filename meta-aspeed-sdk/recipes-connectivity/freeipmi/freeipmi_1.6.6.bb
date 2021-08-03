LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmi-fru;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ZRESEARCH;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.pstdout;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.bmc-watchdog;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmiconsole;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmimonitoring;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmiping;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmipower;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmi-dcmi;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.sunbmc;md5=c03f21cd76ff5caba6b890d1213cbfbb \
                    file://COPYING.ipmiseld;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmidetect;md5=d32239bcb673463ab874e80d47fae504"

SRC_URI = "https://ftp.gnu.org/gnu/freeipmi/freeipmi-${PV}.tar.gz"
SRC_URI[md5sum] = "ad3b6ecbdc4d4b600b90133599dfd51d"
SRC_URI[sha1sum] = "80b4bfc628cf27d5681ba29bc02e11e7835214d5"
SRC_URI[sha256sum] = "cfa30179b44c582e73cf92c2ad0e54fe49f9fd87f7a0889be9dc2db5802e6aab"
SRC_URI[sha384sum] = "ef3981238d7354a8b43388059f2bef218718f9ac3960bcd2012114b11b056fb2d4071560f1bb268be8cc73c80a146a28"
SRC_URI[sha512sum] = "8a7be74bf003b8858c054bac24615f6fba0133e38e6f759ce81ed734a9ab107eb286c70554ad9663062c92eeccf342b80536aac0da74e4ede1ec51eedd497366"

inherit autotools pkgconfig

EXTRA_OECONF = " --without-encryption --without-random-device --with-dont-check-for-root "

PACKAGES =+ " \
    ${PN}-ipmi-fru \
    ${PN}-bmc-watchdog \
    ${PN}-ipmiconsole \
    ${PN}-ipmimonitoring \
    ${PN}-ipmiping \
    ${PN}-ipmipower \
    ${PN}-ipmi-dcmi \
    ${PN}-ipmiseld \
    ${PN}-ipmidetect \
    ${PN}-other \
    "

FILES_${PN}-ipmi-fru = "${sbindir}/ipmi-fru"
FILES_${PN}-bmc-watchdog = "${sbindir}/bmc-watchdog"
FILES_${PN}-ipmiconsole = "${sbindir}/ipmiconsole ${sbindir}/ipmi-console ${libdir}/libipmiconsole.so.2*"
FILES_${PN}-ipmimonitoring = "${sbindir}/ipmimonitoring ${libdir}/libipmimonitoring.so.6*"
FILES_${PN}-ipmiping = "${sbindir}/ipmiping ${sbindir}/ipmi-ping ${sbindir}/rmcpping ${sbindir}/rmcp-ping"
FILES_${PN}-ipmipower = "${sbindir}/ipmipower ${sbindir}/ipmi-power"
FILES_${PN}-ipmi-dcmi = "${sbindir}/ipmi-dcmi"
FILES_${PN}-ipmiseld = "${sbindir}/ipmiseld"
FILES_${PN}-ipmisel = "${sbindir}/ipmi-pef-config ${sbindir}/pef-config ${sbindir}/ipmi-pet ${sbindir}/ipmi-sel"
FILES_${PN}-ipmidetect = "${sbindir}/ipmidetectd ${sbindir}/ipmidetect ${sbindir}/ipmi-detect"
FILES_${PN}-other = " \
    ${sbindir}/bmc-device \
    ${sbindir}/ipmi-chassis \
    ${sbindir}/ipmi-chassis-config \
    ${sbindir}/ipmi-config \
    ${sbindir}/ipmi-locate \
    ${sbindir}/ipmi-oem \
    ${sbindir}/ipmi-sel \
    ${sbindir}/ipmi-sensors \
    ${sbindir}/ipmi-sensors-config \
    ${sysconfdir}/* \
    "
