SUMMARY = "UFM Provisioning Script"
DESCRIPTION = "A Script to emulate BIOS provisioning"

S = "${WORKDIR}"
SRC_URI = " \
    file://ufm \
    "

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

RDEPENDS:${PN} = "i2c-tools"

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/ufm ${D}${bindir}/ufm
}
