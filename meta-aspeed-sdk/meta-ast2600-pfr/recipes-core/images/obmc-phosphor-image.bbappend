FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

DEPENDS:append = " obmc-pfr-image-native intel-pfr-signing-utility-native "
IMAGE_INSTALL:append = " packagegroup-aspeed-pfr-apps "

IMAGE_INSTALL:remove:ast2600-pfr = " \
        webui-vue \
        libmctp \
        entity-manager \
        dbus-sensors \
        "

IMAGE_INSTALL:remove:ast2600-pfr = " \
        packagegroup-oss-apps \
        packagegroup-oss-libs \
        packagegroup-oss-intel-pmci \
        packagegroup-aspeed-obmc-apps \
        packagegroup-aspeed-apps \
        packagegroup-aspeed-crypto \
        packagegroup-aspeed-ssif \
        packagegroup-aspeed-obmc-inband \
        ${@bb.utils.contains('MACHINE_FEATURES', 'ast-ssp', 'packagegroup-aspeed-ssp', '', d)} \
        packagegroup-aspeed-mtdtest \
        packagegroup-aspeed-usbtools \
        ${@bb.utils.contains('DISTRO_FEATURES', 'tpm', \
            bb.utils.contains('MACHINE_FEATURES', 'tpm2', 'packagegroup-security-tpm2', '', d), \
            '', d)} \
        "

# Only install in AST26xx series rofs as the free space of AST2500 rofs is not enough.
IMAGE_INSTALL:remove:ast2600-pfr = " \
        packagegroup-aspeed-ktools \
       "

EXTRA_IMAGE_FEATURES:remove:ast2600-pfr = " \
        nfs-client \
        ${@bb.utils.contains('DISTRO_FEATURES', 'obmc-ubi-fs', 'read-only-rootfs-delayed-postinsts', '', d)} \
        ${@bb.utils.contains('DISTRO_FEATURES', 'phosphor-mmc', 'read-only-rootfs-delayed-postinsts', '', d)} \
        "

inherit image_types_phosphor_aspeed

do_generate_static[depends] += " obmc-phosphor-image:do_generate_static"

python do_generate_static() {
    import subprocess

    bb.build.exec_func("do_mk_static_nor_image", d)

    nor_image = os.path.join(d.getVar('IMGDEPLOYDIR', True),
                             '%s.static.mtd' % d.getVar('IMAGE_NAME', True))

    def _append_image(imgpath, start_kb, finish_kb):
        imgsize = os.path.getsize(imgpath)
        maxsize = (finish_kb - start_kb) * 1024
        bb.debug(1, 'Considering file size=' + str(imgsize) + ' name=' + imgpath)
        bb.debug(1, 'Spanning start=' + str(start_kb) + 'K end=' + str(finish_kb) + 'K')
        bb.debug(1, 'Compare needed=' + str(imgsize) + ' available=' + str(maxsize) + ' margin=' + str(maxsize - imgsize))
        if imgsize > maxsize:
            bb.fatal("Image '%s' is too large!" % imgpath)

        subprocess.check_call(['dd', 'bs=1k', 'conv=notrunc',
                               'seek=%d' % start_kb,
                               'if=%s' % imgpath,
                               'of=%s' % nor_image])

    uboot_offset = int(d.getVar('FLASH_UBOOT_OFFSET', True))

    spl_binary = d.getVar('SPL_BINARY', True)
    if spl_binary:
        _append_image(os.path.join(d.getVar('DEPLOY_DIR_IMAGE', True),
                                   'u-boot-spl.%s' % d.getVar('UBOOT_SUFFIX',True)),
                      int(d.getVar('FLASH_UBOOT_OFFSET', True)),
                      int(d.getVar('FLASH_UBOOT_SPL_SIZE', True)))
        uboot_offset += int(d.getVar('FLASH_UBOOT_SPL_SIZE', True))

    _append_image(os.path.join(d.getVar('DEPLOY_DIR_IMAGE', True),
                               'u-boot.%s' % d.getVar('UBOOT_SUFFIX',True)),
                  uboot_offset,
                  int(d.getVar('FLASH_KERNEL_OFFSET', True)))

    _append_image(os.path.join(d.getVar('DEPLOY_DIR_IMAGE', True),
                               d.getVar('FLASH_KERNEL_IMAGE', True)),
                  int(d.getVar('FLASH_KERNEL_OFFSET', True)),
                  int(d.getVar('FLASH_ROFS_OFFSET', True)))

    _append_image(os.path.join(d.getVar('IMGDEPLOYDIR', True),
                               '%s.%s' % (
                                    d.getVar('IMAGE_LINK_NAME', True),
                                    d.getVar('IMAGE_BASETYPE', True))),
                  int(d.getVar('FLASH_ROFS_OFFSET', True)),
                  int(d.getVar('FLASH_SIZE', True)))

    bb.build.exec_func("do_mk_static_symlinks", d)
}

do_generate_static:append() {
    bb.build.exec_func("do_generate_pfr_image", d)
    bb.build.exec_func("do_generate_signed_pfr_image", d)
}

mk_nor_image() {
    image_dst="$1"
    image_size_kb=$2
    dd if=/dev/zero bs=1k count=$image_size_kb \
        | tr '\000' '\377' > $image_dst
}

do_generate_pfr_image(){
    # Assemble the flash image
    mk_nor_image ${IMGDEPLOYDIR}/image-mtd ${PFR_IMAGE_SIZE}

    dd bs=1k conv=notrunc seek=0 \
        if=${IMGDEPLOYDIR}/${IMAGE_NAME}.static.mtd \
        of=${IMGDEPLOYDIR}/image-mtd
}

# PFR images directory
PFR_IMAGES_DIR = "${DEPLOY_DIR_IMAGE}/pfr_images"

# PFR image generation script directory
PFR_SCRIPT_DIR = "${STAGING_DIR_NATIVE}${bindir}"

# PFR image config directory
PFR_CFG_DIR = "${STAGING_DIR_NATIVE}${datadir}/pfrconfig"

# Temporary hardcode
PFR_BUILD_VER ?= "1"
PFR_BUILD_NUM ?= "2"
PFR_BUILD_HASH ?= "565566"
# 1 = SHA256
# 2 = SHA384
PFR_SHA ?= "1"

do_generate_signed_pfr_image(){
    local manifest_json=${PFR_MANIFEST}
    local pfmconfig_xml=""
    local bmcconfig_xml=""
    local pfm_signed_bin="pfm_signed.bin"
    local signed_cap_bin="bmc_signed_cap.bin"
    local unsigned_cap_bin="bmc_unsigned_cap.bin"
    local unsigned_cap_align_bin="bmc_unsigned_cap.bin_aligned"
    local output_bin="image-mtd-pfr"
    local SIGN_UTILITY=${PFR_SCRIPT_DIR}/intel-pfr-signing-utility

    if [ ${PFR_SHA} == "1" ]; then
        pfmconfig_xml="pfm_config.xml"
        bmcconfig_xml="bmc_config.xml"
    else
        pfmconfig_xml="pfm_config_secp384r1.xml"
        bmcconfig_xml="bmc_config_secp384r1.xml"
    fi

    rm -f ${PFR_IMAGES_DIR}/${unsigned_cap_bin}
    mkdir -p "${PFR_IMAGES_DIR}"
    cd "${PFR_IMAGES_DIR}"

    ${PFR_SCRIPT_DIR}/pfr_image.py -m ${PFR_CFG_DIR}/${manifest_json} \
        -i ${IMGDEPLOYDIR}/image-mtd \
        -n ${PFR_BUILD_VER} \
        -b ${PFR_BUILD_NUM} \
        -h ${PFR_BUILD_HASH} \
        -s ${PFR_SHA} \
        -o ${output_bin}

    # sign the PFM region
    ${SIGN_UTILITY} -c ${PFR_CFG_DIR}/${pfmconfig_xml} -o ${PFR_IMAGES_DIR}/${pfm_signed_bin} ${PFR_IMAGES_DIR}/pfm.bin -v

    # Parsing and Verifying the PFM
    echo "Parsing and Verifying the PFM"
    ${SIGN_UTILITY} -p ${PFR_IMAGES_DIR}/${pfm_signed_bin} -c ${PFR_CFG_DIR}/${pfmconfig_xml}
    if [ $(${SIGN_UTILITY} -p ${PFR_IMAGES_DIR}/${pfm_signed_bin} -c ${PFR_CFG_DIR}/${pfmconfig_xml} 2>&1 | grep "ERR" | wc -c) -gt 0 ]; then
        bbfatal "Verify the PFM failed."
    fi

    # Add the signed PFM to rom image
    dd bs=1k conv=notrunc seek=${PFM_OFFSET_PAGE} if=${PFR_IMAGES_DIR}/${pfm_signed_bin} of=${PFR_IMAGES_DIR}/${output_bin}

    # Create unsigned BMC update capsule - append with 1. pfm_signed, 2. pbc, 3. bmc compressed
    dd if=${PFR_IMAGES_DIR}/${pfm_signed_bin} bs=1k >> ${PFR_IMAGES_DIR}/${unsigned_cap_bin}

    dd if=${PFR_IMAGES_DIR}/pbc.bin bs=1k >> ${PFR_IMAGES_DIR}/${unsigned_cap_bin}

    dd if=${PFR_IMAGES_DIR}/bmc_compressed.bin bs=1k >> ${PFR_IMAGES_DIR}/${unsigned_cap_bin}

    # Sign the BMC update capsule
    ${SIGN_UTILITY} -c ${PFR_CFG_DIR}/${bmcconfig_xml} -o ${PFR_IMAGES_DIR}/${signed_cap_bin} ${PFR_IMAGES_DIR}/${unsigned_cap_bin} -v

    # Parsing and Verifying the BMC update capsule
    echo "Parsing and Verifying the BMC update capsule"
    ${SIGN_UTILITY} -p ${PFR_IMAGES_DIR}/${signed_cap_bin} -c ${PFR_CFG_DIR}/${bmcconfig_xml}
    if [ $(${SIGN_UTILITY} -p ${PFR_IMAGES_DIR}/${signed_cap_bin} -c ${PFR_CFG_DIR}/${bmcconfig_xml} 2>&1 | grep "ERR" | wc -c) -gt 0 ]; then
        bbfatal "Verify the BMC update capsule failed."
    fi

    # Add the signed bmc update capsule to full rom image
    dd bs=1k conv=notrunc seek=${RC_IMAGE_PAGE} if=${PFR_IMAGES_DIR}/${signed_cap_bin} of=${PFR_IMAGES_DIR}/${output_bin}
}
