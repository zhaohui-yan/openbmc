KBRANCH = "aspeed-master-v5.4"
LINUX_VERSION ?= "5.4.62"

# Tag for v00.04.02
SRCREV = "a27958651356a764bcf4dfd332c63c349c8590d4"

require linux-aspeed.inc

DEPENDS += "lzop-native"
