KBRANCH = "aspeed-master-v5.15"
LINUX_VERSION ?= "5.15.0"

# Tag for v00.05.02
SRCREV = "7fe936b7df4fe6f751c02004d43c0e958a4ed793"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

require linux-aspeed.inc

DEPENDS += "lzop-native"
DEPENDS += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-secure-config-native', '', d)}"

SRC_URI:append = " file://ipmi_ssif.cfg "
SRC_URI:append = " file://mtd_test.cfg "
SRC_URI:append = " file://crpyto_manager.cfg "
SRC_URI:append:cypress-s25hx = " file://jffs2_writebuffer.cfg "

