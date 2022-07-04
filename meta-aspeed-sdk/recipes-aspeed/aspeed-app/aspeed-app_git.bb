LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

inherit pkgconfig meson

SRC_URI = " git://gerrit.aspeed.com:29418/aspeed_app.git;protocol=ssh;branch=${BRANCH} "
PV = "1.0+git${SRCPV}"

# Tag for v00.01.07
SRCREV = "22fc1decc8189e6f53070d18ca05857b7eefef79"
BRANCH = "master"

S = "${WORKDIR}/git"

DEPENDS += "openssl"
RDEPENDS:${PN} += "openssl"

FILES:${PN}:append = " /usr/share/* "
