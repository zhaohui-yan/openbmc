SUMMARY = "Packagegroup for Open Source"

PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"

PACKAGES = " \
    ${PN}-apps \
    ${PN}-libs \
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
    "

SUMMARY_${PN}-libs = "Open Source Library"
RDEPENDS_${PN}-libs = " \
    libgpiod \
    "

