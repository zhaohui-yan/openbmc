KBRANCH = "aspeed-master-v5.4"
LINUX_VERSION ?= "5.4.62"

# Tag for v00.04.05
SRCREV = "5698916ba714f870a85affabad80c358b9b0ad62"

require linux-aspeed.inc

DEPENDS += "lzop-native"
DEPENDS += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-secure-config-native', '', d)}"

SRC_URI_append = " file://ipmi_ssif.cfg "
SRC_URI_append = " file://mtd_test.cfg "
