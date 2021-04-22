SUMMARY = "MDIO interface tool for Linux"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM += "file://LICENSE;md5=e8c1458438ead3c34974bc0be3a03ed6"

PV = "1.0+git${SRCPV}"
SRCREV = "72bd5a915ff046a59ce4303c8de672e77622a86c"
SRC_URI = "git://github.com/PieVo/mdio-tool"

S = "${WORKDIR}/git"

inherit cmake

