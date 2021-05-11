KBRANCH = "aspeed-master-v5.4"
LINUX_VERSION ?= "5.4.62"

# Tag for v00.04.00
SRCREV = "6c3d6e8b9f8b4b00887aa7554bf0cc827ac9f8f9"

require linux-aspeed.inc

DEPENDS += "lzop-native"
