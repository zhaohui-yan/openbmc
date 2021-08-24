SUMMARY = "AspeedTech BMC Package Group"

PR = "r2"

inherit packagegroup

PROVIDES = "${PACKAGES}"
RPROVIDES_${PN} = "${PACKAGES}"

PACKAGES_append = " \
    ${PN}-apps \
    ${PN}-ssp \
    ${PN}-crypto \
    ${PN}-ssif \
    ${PN}-mtdtest \
    ${PN}-ktools \
    "

SUMMARY_${PN}-apps = "AspeedTech Test App"
RDEPENDS_${PN}-apps = " \
    aspeed-app \
    "

SUMMARY_${PN}-ssp = "AspeedTech Secondary Service Processor"
RDEPENDS_${PN}-ssp = " \
    ssp \
    "
RRECOMMENDS_${PN}-ssp= " \
    kernel-module-aspeed-ssp \
    "

SUMMARY_${PN}-crypto = "AspeedTech Crypto"
RDEPENDS_${PN}-crypto = " \
    libcrypto \
    libssl \
    openssl \
    openssl-bin \
    openssl-conf \
    openssl-engines \
    openssl-misc \
    "
RRECOMMENDS_${PN}-crypto = " \
    kernel-module-cryptodev \
    "

SUMMARY_${PN}-ssif = "IPMI SMBus System Interface"
RDEPENDS_${PN}-ssif = " \
    "
RRECOMMENDS_${PN}-ssif= " \
    kernel-module-ipmi-msghandler \
    kernel-module-ipmi-ssif \
    kernel-module-ipmi-si \
    kernel-module-ipmi-devintf \
    "

SUMMARY_${PN}-mtdtest = "MTD test utility"
RDEPENDS_${PN}-mtdtest = " \
    "
RRECOMMENDS_${PN}-mtdtest= " \
    kernel-module-mtd-speedtest \
    kernel-module-mtd-stresstest \
    "

SUMMARY_${PN}-ktools = "kernel tools"
RDEPENDS_${PN}-ktools = " \
    "
RRECOMMENDS_${PN}-ktools= " \
    perf \
    "
