SUMMARY = "Set of tools to interact with i3c devices from user space"
HOMEPAGE = "https://github.com/vitor-soares-snps/i3c-tools"

LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://i3ctransfer.c;endline=6;md5=8a1ae5c1aaf128e640de497ceaa9935e"

SRC_URI = "git://github.com/vitor-soares-snps/i3c-tools.git;protocol=https"
SRC_URI_append = " file://meson.build "

PR = "r1"
PV = "1.0+git${SRCPV}"
SRCREV = "2b37323d0de6265e5da3539f29fe34ac317e5b27"

S = "${WORKDIR}/git"

inherit meson

do_configure_prepend () {
	cp ${WORKDIR}/meson.build ${S}
}

