LICENSE = "GPL-2.0+"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

S = "${WORKDIR}/git"

SRC_URI = " git://gerrit.aspeed.com:29418/aspeed_app.git;protocol=ssh;branch=${BRANCH} "
PV = "1.0+git${SRCPV}"

# Build specific revision
# SRCREV = "84d288630f3f73dfb06e11d5e04e44b3899bacf4"

# Build latest revision
SRCREV = "${AUTOREV}"
BRANCH = "develop"
inherit meson

FILES:${PN}:append = " /usr/share/* "
