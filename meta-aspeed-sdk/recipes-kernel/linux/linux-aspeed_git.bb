KBRANCH = "aspeed-master-v5.4"
LINUX_VERSION ?= "5.4.62"

SRCREV = "ad2adc4dc352e47941015c1d9259e564a242dbce"

require linux-aspeed.inc

DEPENDS += "lzop-native"
