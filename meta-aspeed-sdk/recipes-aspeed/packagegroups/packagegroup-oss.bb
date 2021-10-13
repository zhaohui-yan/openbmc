SUMMARY = "Packagegroup for Open Source"

PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"

PACKAGES = " \
    ${PN}-apps \
    ${PN}-libs \
    "

SUMMARY:${PN}-apps = "Open Source Applications"
RDEPENDS:${PN}-apps = " \
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
    stressapptest \
    e2fsprogs-mke2fs \
    nvme-cli \
    ${@d.getVar('PREFERRED_PROVIDER_u-boot-fw-utils', True) or 'u-boot-fw-utils'} \
    "

RDEPENDS:${PN}-apps:append:aspeed-g6 = " \
    fio \
    "

SUMMARY:${PN}-libs = "Open Source Library"
RDEPENDS:${PN}-libs = " \
    libgpiod \
    libgpiod-tools \
    "
