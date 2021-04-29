SUMMARY = "Secure-boot utilities for ASPEED BMC SoCs"
HOMEPAGE = "https://github.com/AspeedTech-BMC/socsec/"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d50b901333b4eedfee074ebcd6a6d611"

SRC_URI = "git://github.com/AspeedTech-BMC/socsec.git/;protocol=https;branch=develop"

PV = "2.0+git${SRCPV}"
# Tag for v00.02.00
SRCREV = "e15bf0a1d97bc5480efb8bd66e8c336fe72787a1"

S = "${WORKDIR}/git"

inherit python3native setuptools3

RDEPENDS_${PN} += "python3-bitarray"
RDEPENDS_${PN} += "python3-core"
RDEPENDS_${PN} += "python3-hexdump"
RDEPENDS_${PN} += "python3-jsonschema"
RDEPENDS_${PN} += "python3-jstyleson"
RDEPENDS_${PN} += "python3-pycryptodome"

BBCLASSEXTEND = "native nativesdk"
