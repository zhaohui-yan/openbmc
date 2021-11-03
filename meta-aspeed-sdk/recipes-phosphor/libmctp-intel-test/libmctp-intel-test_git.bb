SUMMARY = "libmctp:intel"
DESCRIPTION = "Implementation of MCTP(DMTF DSP0236)"

SRC_URI = "git://github.com/Intel-BMC/libmctp.git;protocol=https \
           file://0001-support-to-build-over-smbus-and-astpcie-test-program.patch \
           file://0002-Remove-unknown-i2c-smbus.h.patch \
           file://0003-Costomize-buffer-size.patch \
           file://0004-i2c-change-max-payload-size-to-255.patch \
           file://mctp-astpcie-test.c \
           file://mctp-astpcie-test.h \
           file://mctp-smbus-test.c \
           file://mctp-smbus-test.h \
          "
SRCREV = "52117fa04e6afabe8eb1285c702f1400fecfb992"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

LICENSE = "Apache-2.0 | GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0d30807bb7a4f16d36e96b78f9ed8fae"

inherit cmake

DEPENDS += "i2c-tools"

do_configure:prepend() {
    install -m 0644 ${WORKDIR}/mctp-astpcie-test.c ${S}/tests
    install -m 0644 ${WORKDIR}/mctp-astpcie-test.h ${S}/tests
    install -m 0644 ${WORKDIR}/mctp-smbus-test.c ${S}/tests
    install -m 0644 ${WORKDIR}/mctp-smbus-test.h ${S}/tests
}
