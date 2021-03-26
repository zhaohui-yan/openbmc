FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

DEPENDS_append_df-phosphor-mmc = " python3-native aspeed-image-tools-native "

SRC_URI_append_df-phosphor-mmc = " file://u-boot-env-ast2600.txt "

do_deploy_append_df-phosphor-mmc() {
    gen_emmc_boot_image.py ${SPL_BINARYNAME} emmc_${SPL_BINARYNAME}

    # Generate image-u-boot for boot from eMMC
    dd bs=1k count=1024 \
        if=/dev/zero of=emmc_image-u-boot
    dd bs=1k count=64 conv=notrunc \
        if=emmc_${SPL_BINARYNAME} of=emmc_image-u-boot
    dd bs=1k seek=64 conv=notrunc \
        if=${UBOOT_BINARY} of=emmc_image-u-boot
}
