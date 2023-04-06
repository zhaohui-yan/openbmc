SUMMARY = "The xdma-test application performs DMA operations between the BMC and the host."
HOMEPAGE = "https://github.com/eddiejames/xdma-test"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

INSANE_SKIP:${PN} = "ldflags"

SRC_URI = "git://github.com/eddiejames/xdma-test.git;protocol=https;branch=master"

PV = "1.0+git${SRCPV}"
SRCREV = "caf176d53488433e2685ec082ac3d16bed9c1dc2"

S = "${WORKDIR}/git"
B = "${S}"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/xdma-test ${D}${bindir}/xdma-test
}

FILES:${PN} = "/usr/bin/xdma-test"

