
SRC_URI = "git://github.com/PieVo/mdio-tool"

LIC_FILES_CHKSUM += "file://LICENSE;md5=e8c1458438ead3c34974bc0be3a03ed6"

LICENSE = "GPL-2.0"

inherit cmake

S = "${WORKDIR}/git"
SRCREV = "${AUTOREV}"
PV = "1.0+git${SRCPV}"
