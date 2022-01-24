LICENSE = "GPL-2.0+"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

S = "${WORKDIR}/git"

SRC_URI = " git://github.com/AspeedTech-BMC/aspeed_app.git;protocol=https;branch=${BRANCH} "
PV = "1.0+git${SRCPV}"

# Tag for v00.01.04
SRCREV = "08217ae2099bd99dab52f7fa1407bca7f04acfd4"
BRANCH = "master"
inherit meson

FILES:${PN}:append = " /usr/share/* "
