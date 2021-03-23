SUMMARY = "Two partition MTD image with u-boot and kernel"
HOMEPAGE = "https://github.com/openbmc/meta-aspeed"
LICENSE = "MIT"

inherit deploy

UBOOT_SUFFIX ?= "bin"
ASPEED_IMAGE_UBOOT_OFFSET_KB ?= "0"
ASPEED_IMAGE_UBOOT_SPL_SIZE_KB ?= "64"
ASPEED_IMAGE_KERNEL_OFFSET_KB ?= "1024"
ASPEED_IMAGE_SIZE_KB ?= "16384"
ASPEED_IMAGE_KERNEL_IMAGE ?= "fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}"
ASPEED_IMAGE_NAME ?= "all.bin"

do_compile() {
    uboot_offset=${ASPEED_IMAGE_UBOOT_OFFSET_KB}

    dd if=/dev/zero bs=1k count=${ASPEED_IMAGE_SIZE_KB} | \
        tr '\000' '\377' > ${B}/aspeed-norootfs.bin

    if [ ! -z ${SPL_BINARY} ]; then
        dd bs=1k conv=notrunc seek=${ASPEED_IMAGE_UBOOT_OFFSET_KB} \
            if=${DEPLOY_DIR_IMAGE}/u-boot-spl.${UBOOT_SUFFIX} \
            of=${B}/aspeed-norootfs.bin
        uboot_offset=${ASPEED_IMAGE_UBOOT_SPL_SIZE_KB}
    fi

    dd bs=1k conv=notrunc seek=${uboot_offset} \
        if=${DEPLOY_DIR_IMAGE}/u-boot.${UBOOT_SUFFIX} \
        of=${B}/aspeed-norootfs.bin \

    dd bs=1k conv=notrunc seek=${ASPEED_IMAGE_KERNEL_OFFSET_KB} \
        if=${DEPLOY_DIR_IMAGE}/${ASPEED_IMAGE_KERNEL_IMAGE} \
        of=${B}/aspeed-norootfs.bin
}

do_deploy() {
    install -m644 -D ${B}/aspeed-norootfs.bin ${DEPLOYDIR}/${ASPEED_IMAGE_NAME}
}

do_compile[depends] = "virtual/kernel:do_deploy u-boot:do_deploy"
do_fetch[noexec] = "1"
do_unpack[noexec] = "1"
do_patch[noexec] = "1"
do_configure[noexec] = "1"
do_install[noexec] = "1"
deltask do_populate_sysroot
do_package[noexec] = "1"
deltask do_package_qa
do_packagedata[noexec] = "1"
deltask do_package_write_ipk
deltask do_package_write_deb
deltask do_package_write_rpm
addtask deploy before do_build after do_compile
