HOMEPAGE = "https://github.com/AspeedTech-BMC/u-boot"
SECTION = "bootloaders"
DEPENDS += "flex-native bison-native"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=30503fd321432fc713238f582193b78e"
PE = "1"

BRANCH="aspeed-dev-v2019.04"
SRC_URI = "git://192.168.10.13:29418/u-boot.git;protocol=ssh;branch=${BRANCH}"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"
UBOOT_LOCALVERSION = "-${SRCPV}"
