LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

S = "${WORKDIR}/git"

SRC_URI = " git://github.com/AspeedTech-BMC/aspeed_app.git;protocol=https;branch=${BRANCH} "
PV = "1.0+git${SRCPV}"

# Tag for v00.01.06
SRCREV = "265fabcee4777c149a1a6fe18d3fe3db8516e5d2"
BRANCH = "master"
inherit meson

FILES:${PN}:append = " /usr/share/* "
