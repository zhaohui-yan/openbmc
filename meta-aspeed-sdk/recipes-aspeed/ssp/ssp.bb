SUMMARY = "The Secondary Service Processor (SSP) firmware"
DESCRIPTION = "The Secondary Service Processor (SSP) is an ARM Cortex-M3 r2p1 processor \
which is aimed at monitoring/controlling the peripherals to offload the primary processor Cortex-A7"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${ASPEEDSDKBASE}/LICENSE;md5=796e0355fcfe2a3345d663c4153c9e42"

PR = "r0"

SSP_FIRMWARE = "ast2600_ssp.bin"

SRC_URI = "file://${SSP_FIRMWARE};subdir=${S}"

do_patch[noexec] = "1"
do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install () {
    install -d ${D}/${baselib}/firmware
    install -m 644 ${S}/${SSP_FIRMWARE} ${D}/${baselib}/firmware
}

FILES_${PN} += "${baselib}/firmware/*"
