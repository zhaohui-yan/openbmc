LICENSE = "GPL-2.0+"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

S = "${WORKDIR}/git"

SRC_URI = " git://github.com/AspeedTech-BMC/aspeed_app.git;protocol=https;branch=${BRANCH} "
PV = "1.0+git${SRCPV}"

# Tag for v00.01.03
SRCREV = "e41ca51cbd69e739119d60df46daa2f8530e5094"
BRANCH = "master"
inherit meson

FILES:${PN}:append = " /usr/share/* "
