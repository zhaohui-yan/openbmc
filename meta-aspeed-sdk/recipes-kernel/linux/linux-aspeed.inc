DESCRIPTION = "Linux kernel for Aspeed"
SECTION = "kernel"
LICENSE = "GPLv2"

PROVIDES += "virtual/kernel"

KCONFIG_MODE="--alldefconfig"
KERNEL_VERSION_SANITY_SKIP="1"

KSRC = "git://192.168.10.13:29418/linux.git;protocol=ssh;branch=${KBRANCH}"

SRC_URI = "${KSRC}"
SRC_URI:append:ast-abr = " file://0001-abr-alternate-flash-layout.patch "

LINUX_VERSION_EXTENSION ?= "-${SRCREV}"

PV = "${LINUX_VERSION}+git${SRCPV}"

inherit kernel gitpkgv
require recipes-kernel/linux/linux-yocto.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"

# Do not install the kernel image into rootfs at /boot directory
RDEPENDS:${KERNEL_PACKAGE_NAME}-base = ""

PKGVTAG ="${GITPKGVTAG}"

do_set_local_version() {
    rm -f ${S}/.scmversion
    rm -f ${B}/.scmversion

    if [[ "${PKGVTAG}" == "None" ||  "${PKGVTAG}" == "0+0" ]]; then
        echo "not a git repository!!!"
    elif [[ "${PKGVTAG}" == "0.0-"*"-g"* ]] ; then
        version="$(git -C ${S} rev-parse --verify --short HEAD)"
        echo "-dirty-${version}" > ${S}/.scmversion
        echo "-dirty-${version}" > ${B}/.scmversion
    else
        version="$(git -C ${S} describe --tags --exact-match HEAD)"

        if echo "${version}" | grep -q "devtool"; then
             version_ext="$(git -C ${S} rev-parse --verify --short HEAD)"
             echo "-${version}-${version_ext}" > ${S}/.scmversion
             echo "-${version}-${version_ext}" > ${B}/.scmversion
        else
            echo "-${version}" > ${S}/.scmversion
            echo "-${version}" > ${B}/.scmversion
        fi
    fi
}

addtask set_local_version before do_configure after do_kernel_configme
