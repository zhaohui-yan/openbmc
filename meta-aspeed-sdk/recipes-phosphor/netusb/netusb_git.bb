SUMMARY = "Turn On Network USB gadget"
DESCRIPTION = "Script to turn on network usb gadget after BMC is ready"

S = "${WORKDIR}"
SRC_URI = "file://netusb.sh \
           file://netusb.service \
          "

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit systemd

do_install() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/netusb.service ${D}${systemd_system_unitdir}
    install -d ${D}${bindir}
    install -m 0755 ${S}/netusb.sh ${D}/${bindir}/netusb.sh
}

SYSTEMD_SERVICE_${PN} += " netusb.service"
SYSTEMD_AUTO_ENABLE_${PN} = "disable"
