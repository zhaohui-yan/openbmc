SUMMARY = "Set of tools to interact with i3c devices from user space"
HOMEPAGE = "https://github.com/vitor-soares-snps/i3c-tools"

LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://i3ctransfer.c;endline=6;md5=8a1ae5c1aaf128e640de497ceaa9935e"

SRC_URI = "git://github.com/vitor-soares-snps/i3c-tools.git;protocol=https"

PR = "r1"
PV = "1.0+git${SRCPV}"
SRCREV = "5d752038c72af8e011a2cf988b1476872206e706"

S = "${WORKDIR}/git"

inherit meson

