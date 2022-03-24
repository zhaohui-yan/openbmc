SUMMARY = "ncsi-netlink"
DESCRIPTION = "ncsi-netlink"
SECTION = "base"
TARGET_CC_ARCH += "${LDFLAGS}"
PR = "r1"
LICENSE = "GPL-2.0"

SRC_URI = "git://github.com:/sammj/ncsi-netlink;protocol=https;branch=master"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"
SRCREV = "333ffc4caef79017a9d5d56d69df457b4fb4fcc0"

DEPENDS += "libnl"

PV = "1.0+git${SRCPV}"
S = "${WORKDIR}/git"

inherit base

do_compile() {
    ${CC} ncsi-netlink.c -o ncsi-netlink -Wall -I${STAGING_INCDIR}/libnl3 -L${STAGING_LIBDIR} -l:libnl-genl-3.so.200 -l:libnl-3.so.200
}

do_install() {
    install -d ${D}/usr/bin
    install -m 755 ncsi-netlink ${D}/usr/bin
}

do_configure[noexec] = "1"
