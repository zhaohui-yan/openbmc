SUMMARY = "AspeedTech BMC PFR Package Group"

PR = "r1"

PACKAGE_ARCH="${TUNE_PKGARCH}"

inherit packagegroup

PROVIDES = "${PACKAGES}"
RPROVIDES:${PN} = "${PACKAGES}"

PACKAGES:append = " \
    ${PN}-apps \
    ${PN}-cerberus \
    "

SUMMARY:${PN}-apps = "AspeedTech PFR App package"
RDEPENDS:${PN}-apps = " \
    aspeed-pfr-tool \
    "

SUMMARY:${PN}-cerberus = "AspeedTech Cerberus PFR package"
RDEPENDS:${PN}-cerberus = " \
    cerberus-pfr-provision-image \
    "

