LICENSE = "GPL-2.0+"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

S = "${WORKDIR}/git"

SRC_URI = " git://gerrit.aspeed.com:29418/aspeed_app.git;protocol=ssh;branch=${BRANCH} "
PV = "1.0+git${SRCPV}"

# Tag for v00.01.05
SRCREV = "1fa803c9d93350f92b99f0f973b55eac74dbd4d6"
BRANCH = "master"
inherit meson

FILES:${PN}:append = " /usr/share/* "
