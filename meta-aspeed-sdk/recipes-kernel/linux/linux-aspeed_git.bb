KBRANCH = "aspeed-dev-v5.4"
LINUX_VERSION ?= "5.4.62"

SRCREV = "${AUTOREV}"

require linux-aspeed.inc

DEPENDS += "lzop-native"
DEPENDS += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-secure-config-native', '', d)}"

SRC_URI_append = " file://ipmi_ssif.cfg "
SRC_URI_append = " file://mtd_test.cfg "
SRC_URI_append_aspeed-g6 = " file://host_bmc_dev.cfg "
