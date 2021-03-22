SUMMARY = "OpenBMC for AspeedTech - Applications"

PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"

PACKAGES = " \
        ${PN}-app \
	"

SUMMARY_${PN}-app = "AspeedTech Test App"
RDEPENDS_${PN}-app = " \
        ast-app \
        "
