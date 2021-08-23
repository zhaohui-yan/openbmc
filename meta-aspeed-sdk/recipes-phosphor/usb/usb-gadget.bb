SUMMARY = "Turn On USB gadget"
DESCRIPTION = "Script to turn on usb gadget after BMC is ready"

S = "${WORKDIR}"
SRC_URI = "file://netusb.sh \
           file://netusb.service \
           file://uart.sh \
           file://ms.sh \
           file://keyboard.sh \
           file://mouse.sh \
	   file://hid_gadget_app \
          "

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit systemd

do_install() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/netusb.service ${D}${systemd_system_unitdir}
    install -d ${D}${bindir}
    install -m 0755 ${S}/netusb.sh ${D}/${bindir}/netusb.sh
    install -m 0755 ${S}/uart.sh ${D}/${bindir}/uart.sh
    install -m 0755 ${S}/ms.sh ${D}/${bindir}/ms.sh
    install -m 0755 ${S}/keyboard.sh ${D}/${bindir}/keyboard.sh
    install -m 0755 ${S}/mouse.sh ${D}/${bindir}/mouse.sh
    install -m 0755 ${S}/hid_gadget_app ${D}/${bindir}/hid_gadget_app
}

SYSTEMD_SERVICE_${PN} += " netusb.service"
SYSTEMD_AUTO_ENABLE_${PN} = "disable"
