DESCRIPTION = "Linux kernel for Aspeed"
SECTION = "kernel"
LICENSE = "GPLv2"

PROVIDES += "virtual/kernel"

KCONFIG_MODE="--alldefconfig"
KERNEL_VERSION_SANITY_SKIP="1"

KSRC = "git://192.168.10.13:29418/linux.git;protocol=ssh;branch=${KBRANCH}"

SRC_URI = "${KSRC}"

LINUX_VERSION_EXTENSION ?= "-${SRCREV}"

PV = "${LINUX_VERSION}+git${SRCPV}"

inherit kernel
require recipes-kernel/linux/linux-yocto.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"

# Do not install the kernel image into rootfs at /boot directory
RDEPENDS_${KERNEL_PACKAGE_NAME}-base = ""
