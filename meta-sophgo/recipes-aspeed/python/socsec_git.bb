SUMMARY = "Secure-boot utilities for ASPEED BMC SoCs"
HOMEPAGE = "https://github.com/AspeedTech-BMC/socsec/"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d50b901333b4eedfee074ebcd6a6d611"

SRC_URI = "git://github.com/AspeedTech-BMC/socsec.git;protocol=https;branch=master"

PV = "v02.00.00+git${SRCPV}"
# Tag for v02.00.00
SRCREV = "32fb3f1b2d0955233122d0ffd53cb7833d97eb87"

S = "${WORKDIR}/git"

inherit python3native setuptools3

DEPENDS += "python3-bitarray"
DEPENDS += "python3-jsonschema"
DEPENDS += "python3-jstyleson"
DEPENDS += "python3-pycryptodome"
DEPENDS += "python3-ecdsa"

RDEPENDS:${PN} += "python3-bitarray"
RDEPENDS:${PN} += "python3-core"
RDEPENDS:${PN} += "python3-jsonschema"
RDEPENDS:${PN} += "python3-jstyleson"
RDEPENDS:${PN} += "python3-pycryptodome"
RDEPENDS:${PN} += "python3-ecdsa"

BBCLASSEXTEND = "native nativesdk"
