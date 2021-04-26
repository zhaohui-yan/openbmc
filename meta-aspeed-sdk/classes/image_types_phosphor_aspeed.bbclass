FLASH_UBOOT_SPL_IMAGE ?= "u-boot-spl"
FLASH_UBOOT_IMAGE ?= "u-boot"

do_generate_image_uboot_file() {
    image_dst="$1"
    uboot_offset=${FLASH_UBOOT_OFFSET}

    if [ ! -z ${SPL_BINARY} ]; then
        dd bs=1k conv=notrunc seek=${FLASH_UBOOT_OFFSET} \
            if=${DEPLOY_DIR_IMAGE}/${FLASH_UBOOT_SPL_IMAGE}.${UBOOT_SUFFIX} \
            of=${image_dst}
        uboot_offset=${FLASH_UBOOT_SPL_SIZE}
    fi

    dd bs=1k conv=notrunc seek=${uboot_offset} \
        if=${DEPLOY_DIR_IMAGE}/${FLASH_UBOOT_IMAGE}.${UBOOT_SUFFIX} \
        of=${image_dst}
}

# Include the full u-boot-spl and u-boot in the final static image
python do_generate_static_append() {
    uboot_offset = int(d.getVar('FLASH_UBOOT_OFFSET', True))
    spl_binary = d.getVar('SPL_BINARY', True)
    if spl_binary:
        _append_image(os.path.join(d.getVar('DEPLOY_DIR_IMAGE', True),
                                   '%s.%s' % (
                                    d.getVar('FLASH_UBOOT_SPL_IMAGE', True),
                                    d.getVar('UBOOT_SUFFIX', True))),
                      int(d.getVar('FLASH_UBOOT_OFFSET', True)),
                      int(d.getVar('FLASH_UBOOT_SPL_SIZE', True)))
        uboot_offset += int(d.getVar('FLASH_UBOOT_SPL_SIZE', True))

    _append_image(os.path.join(d.getVar('DEPLOY_DIR_IMAGE', True),
                               '%s.%s' % (
                                    d.getVar('FLASH_UBOOT_IMAGE', True),
                                    d.getVar('UBOOT_SUFFIX', True))),
                  uboot_offset,
                  int(d.getVar('FLASH_KERNEL_OFFSET', True)))
}

do_make_ubi_append() {
    # Concatenate the uboot and ubi partitions
    uboot_offset=${FLASH_UBOOT_OFFSET}

    if [ ! -z ${SPL_BINARY} ]; then
        dd bs=1k conv=notrunc seek=${FLASH_UBOOT_OFFSET} \
            if=${DEPLOY_DIR_IMAGE}/${FLASH_UBOOT_SPL_IMAGE}.${UBOOT_SUFFIX} \
            of=${IMGDEPLOYDIR}/${IMAGE_NAME}.ubi.mtd
        uboot_offset=${FLASH_UBOOT_SPL_SIZE}
    fi

    dd bs=1k conv=notrunc seek=${uboot_offset} \
        if=${DEPLOY_DIR_IMAGE}/${FLASH_UBOOT_IMAGE}.${UBOOT_SUFFIX} \
        of=${IMGDEPLOYDIR}/${IMAGE_NAME}.ubi.mtd
}

do_generate_ext4_tar_ast-secure() {
	# Generate the U-Boot image
	mk_empty_image_zeros image-u-boot ${MMC_UBOOT_SIZE}
	do_generate_image_uboot_file image-u-boot

	# Generate a compressed ext4 filesystem with the fitImage file in it to be
	# flashed to the boot partition of the eMMC
	install -d boot-image

	# Overrides the FIT image file name in image-kernel filesystem
	install -m 644 ${DEPLOY_DIR_IMAGE}/${FLASH_KERNEL_IMAGE} boot-image/s_fitImage
	mk_empty_image_zeros boot-image.${FLASH_EXT4_BASETYPE} ${MMC_BOOT_PARTITION_SIZE}
	mkfs.ext4 -F -i 4096 -d boot-image boot-image.${FLASH_EXT4_BASETYPE}
	# Error codes 0-3 indicate successfull operation of fsck
	fsck.ext4 -pvfD boot-image.${FLASH_EXT4_BASETYPE} || [ $? -le 3 ]
	zstd -f -k -T0 -c ${ZSTD_COMPRESSION_LEVEL} boot-image.${FLASH_EXT4_BASETYPE} > boot-image.${FLASH_EXT4_BASETYPE}.zst

	# Generate the compressed ext4 rootfs
	zstd -f -k -T0 -c ${ZSTD_COMPRESSION_LEVEL} ${IMGDEPLOYDIR}/${IMAGE_LINK_NAME}.${FLASH_EXT4_BASETYPE} > ${IMAGE_LINK_NAME}.${FLASH_EXT4_BASETYPE}.zst

	ln -sf boot-image.${FLASH_EXT4_BASETYPE}.zst image-kernel
	ln -sf ${IMAGE_LINK_NAME}.${FLASH_EXT4_BASETYPE}.zst image-rofs
	ln -sf ${IMGDEPLOYDIR}/${IMAGE_LINK_NAME}.rwfs.${FLASH_EXT4_OVERLAY_BASETYPE} image-rwfs
	ln -sf ${S}/MANIFEST MANIFEST
	ln -sf ${S}/publickey publickey

	hostfw_update_file="${DEPLOY_DIR_IMAGE}/hostfw/update/image-hostfw"
	if [ -e "${hostfw_update_file}" ]; then
		ln -sf "${hostfw_update_file}" image-hostfw
		make_signatures image-u-boot image-kernel image-rofs image-rwfs MANIFEST publickey image-hostfw
		make_tar_of_images ext4.mmc MANIFEST publickey ${signature_files} image-hostfw
	else
		make_signatures image-u-boot image-kernel image-rofs image-rwfs MANIFEST publickey
		make_tar_of_images ext4.mmc MANIFEST publickey ${signature_files}
	fi
}

do_make_ubi[depends] += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-image-secureboot:do_deploy', '', d)}"
do_generate_ubi_tar[depends] += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-image-secureboot:do_deploy', '', d)}"
do_generate_static_tar[depends] += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-image-secureboot:do_deploy', '', d)}"
do_generate_ext4_tar[depends] += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-image-secureboot:do_deploy', '', d)}"
do_generate_static[depends] += "${@bb.utils.contains('MACHINE_FEATURES', 'ast-secure', 'aspeed-image-secureboot:do_deploy', '', d)}"
