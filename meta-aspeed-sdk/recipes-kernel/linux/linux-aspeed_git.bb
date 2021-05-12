KBRANCH = "aspeed-master-v5.4"
LINUX_VERSION ?= "5.4.62"

# Tag for v00.04.01
SRCREV = "3d1c326604ecb772cc23b67cd1c1af165eda5791"

require linux-aspeed.inc

DEPENDS += "lzop-native"
