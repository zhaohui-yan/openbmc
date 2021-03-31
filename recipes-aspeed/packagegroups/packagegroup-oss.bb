SUMMARY = "Packagegroup for Open Source"

PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"

PACKAGES = " \
        ${PN}-apps \
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
        "
