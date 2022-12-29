SUMMARY = "SPDM Emulator"
DESCRIPTION = "This spdm-emu is a sample SPDM emulator implementation using libspdm"

LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE.md;md5=d6578ce21490af4b24f9ed3392884549"
inherit cmake
inherit obmc-phosphor-systemd

SRC_URI = " \
	gitsm://github.com/DMTF/spdm-emu;protocol=https;branch=main \
	file://0001-Support-Yocto-build-and-LibMCTP-Intel-with-SPDM-EMU.patch \
	file://slave-mqueue.sh \
	file://bmc-attestation-emu.service \
	file://pch-attestation-emu.service \
	file://ecp384 \
	"

PV = "2.2.0+git${SRCPV}"
SRCREV = "aef708d2016a17722ff9eddb7f312fb5ac8e4f7e"

S = "${WORKDIR}/git"

EXTRA_OECMAKE = "-DARCH=arm -DTOOLCHAIN=YOCTO -DTARGET=Debug -DCRYPTO=mbedtls"

DEPENDS += " \
	libmctp-intel \
	"

SYSTEMD_SERVICE:${PN} = "bmc-attestation-emu.service pch-attestation-emu.service"

FILES:${PN}:append = " ${datadir}/spdm-emu"

do_install:append () {
	install -d ${D}${systemd_system_unitdir}
	install -m 0644 ${WORKDIR}/bmc-attestation-emu.service ${D}${systemd_system_unitdir}/
	install -m 0644 ${WORKDIR}/pch-attestation-emu.service ${D}${systemd_system_unitdir}/

	install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/slave-mqueue.sh ${D}${bindir}

	install -d ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/bundle_requester.certchain1.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/bundle_requester.certchain.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/bundle_responder.certchain1.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/bundle_responder.certchain.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca1.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca1.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca1.key ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca1.key.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca.key ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/ca.key.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester1.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester1.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester.key ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester.key.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester.key.p8 ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_requester.req ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder1.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder1.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder.key ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder.key.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder.key.p8 ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/end_responder.req ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/inter1.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/inter1.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/inter.cert ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/inter.cert.der ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/inter.key ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/inter.req ${D}${datadir}/spdm-emu/ecp384
	install -m 0644 ${WORKDIR}/ecp384/param.pem ${D}${datadir}/spdm-emu/ecp384
}


