DESCRIPTION = "Linux kernel for Aspeed"
SECTION = "kernel"
LICENSE = "GPLv2"

PROVIDES += "virtual/kernel"

KCONFIG_MODE="--alldefconfig"
KERNEL_VERSION_SANITY_SKIP="1"

KSRC = "git://github.com/AspeedTech-BMC/linux.git;protocol=https;branch=${KBRANCH}"
SRC_URI = "${KSRC}"

LINUX_VERSION_EXTENSION ?= "-${SRCREV}"

PV = "${LINUX_VERSION}+git${SRCPV}"

inherit kernel
require recipes-kernel/linux/linux-yocto.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"

# Do not install the kernel image into rootfs at /boot directory
RDEPENDS_${KERNEL_PACKAGE_NAME}-base = ""

do_set_local_version() {
    is_tag="$(git -C ${S} name-rev --name-only --tags HEAD)"

    if [ $? -ne 0 ] ; then
        echo "not a git repository!!!"
    else
        # Remove previous CONFIG_LOCALVERSION
        sed -i '/CONFIG_LOCALVERSION/d' ${B}/.config

        if test "x${is_tag}" = "xundefined"; then
            version="$(git -C ${S} rev-parse --verify --short HEAD)"
            echo "CONFIG_LOCALVERSION="\"-dirty-${version}\" >> ${B}/.config
        else
            version="$(git -C ${S} describe --tags --exact-match HEAD)"

            if echo "${version}" | grep -q "devtool"; then
                version_ext="$(git -C ${S} rev-parse --verify --short HEAD)"
                echo "CONFIG_LOCALVERSION="\"-${version}-${version_ext}\" >> ${B}/.config
            else
                echo "CONFIG_LOCALVERSION="\"-${version}\" >> ${B}/.config
            fi
        fi
    fi
}

addtask set_local_version before do_configure after do_kernel_configme
