SUMMARY = "Packagegroup for Open Source"

PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"

PACKAGES = " \
    ${PN}-apps \
    ${PN}-libs \
    ${PN}-obmc-apps \
    "

SUMMARY_${PN}-apps = "Open Source Applications"
RDEPENDS_${PN}-apps = " \
    mdio-tool \
    gperf \
    iperf3 \
    pciutils \
    ethtool \
    mmc-utils \
    memtester \
    i3c-tools \
    i2c-tools \
    xdma-test \
    libpeci \
    dhrystone \
    nbd-client \
    iozone3 \
    ncsi-netlink \
    hdparm \
    fio \
    stressapptest \
    ${@d.getVar('PREFERRED_PROVIDER_u-boot-fw-utils', True) or 'u-boot-fw-utils'} \
    "

SUMMARY_${PN}-libs = "Open Source Library"
RDEPENDS_${PN}-libs = " \
    libgpiod \
    libgpiod-tools \
    "

SUMMARY_${PN}-obmc-apps = "Open Source Applications for OpenBMC Image"
RDEPENDS_${PN}-obmc-apps = " \
    freeipmi \
    ipmitool \
    at-scale-debug \
    "
