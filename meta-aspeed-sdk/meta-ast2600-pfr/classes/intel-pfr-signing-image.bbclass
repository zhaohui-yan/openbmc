DEPENDS:append = " obmc-pfr-image-native \
                   intel-pfr-signing-utility-native \
                 "

do_generate_static:append() {
    bb.build.exec_func("do_generate_signed_pfr_image", d)
}

# PFR images directory
PFR_DEPLOY_IMAGES_DIR = "${IMGDEPLOYDIR}/pfr_images"
PFR_IMAGES_DIR = "${S}/pfr_images"

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
PFR_SHA ?= "2"

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

    if [ "${PFR_SHA}" == "1" ]; then
        pfmconfig_xml="pfm_config.xml"
        bmcconfig_xml="bmc_config.xml"
    else
        pfmconfig_xml="pfm_config_secp384r1.xml"
        bmcconfig_xml="bmc_config_secp384r1.xml"
    fi

    rm -rf ${PFR_IMAGES_DIR}
    mkdir -p ${PFR_IMAGES_DIR}
    cd ${PFR_IMAGES_DIR}

    # Assemble the flash image
    mk_empty_image ${PFR_IMAGES_DIR}/image-mtd ${PFR_IMAGE_SIZE}

    dd bs=1k conv=notrunc seek=0 \
        if=${IMGDEPLOYDIR}/${IMAGE_NAME}.static.mtd \
        of=${PFR_IMAGES_DIR}/image-mtd

    python3 ${PFR_SCRIPT_DIR}/pfr_image.py -m ${PFR_CFG_DIR}/${manifest_json} \
        -i ${PFR_IMAGES_DIR}/image-mtd \
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
    #if [ $(${SIGN_UTILITY} -p ${PFR_IMAGES_DIR}/${pfm_signed_bin} -c ${PFR_CFG_DIR}/${pfmconfig_xml} 2>&1 | grep "ERR" | wc -c) -gt 0 ]; then
    #    bbfatal "Verify the PFM failed."
    #fi

    # Add the signed PFM to rom image
    dd bs=1k conv=notrunc seek=${PFM_OFFSET_PAGE} \
        if=${PFR_IMAGES_DIR}/${pfm_signed_bin} \
        of=${PFR_IMAGES_DIR}/${output_bin}

    # Create unsigned BMC update capsule - append with 1. pfm_signed, 2. pbc, 3. bmc compressed
    dd if=${PFR_IMAGES_DIR}/${pfm_signed_bin} bs=1k >> ${PFR_IMAGES_DIR}/${unsigned_cap_bin}

    dd if=${PFR_IMAGES_DIR}/pbc.bin bs=1k >> ${PFR_IMAGES_DIR}/${unsigned_cap_bin}

    dd if=${PFR_IMAGES_DIR}/bmc_compressed.bin bs=1k >> ${PFR_IMAGES_DIR}/${unsigned_cap_bin}

    # Sign the BMC update capsule
    ${SIGN_UTILITY} -c ${PFR_CFG_DIR}/${bmcconfig_xml} -o ${PFR_IMAGES_DIR}/${signed_cap_bin} ${PFR_IMAGES_DIR}/${unsigned_cap_bin} -v

    # Parsing and Verifying the BMC update capsule
    echo "Parsing and Verifying the BMC update capsule"
    ${SIGN_UTILITY} -p ${PFR_IMAGES_DIR}/${signed_cap_bin} -c ${PFR_CFG_DIR}/${bmcconfig_xml}
    #if [ $(${SIGN_UTILITY} -p ${PFR_IMAGES_DIR}/${signed_cap_bin} -c ${PFR_CFG_DIR}/${bmcconfig_xml} 2>&1 | grep "ERR" | wc -c) -gt 0 ]; then
    #    bbfatal "Verify the BMC update capsule failed."
    #fi

    # Add the signed bmc update capsule to full rom image
    dd bs=1k conv=notrunc seek=${RC_IMAGE_PAGE} \
        if=${PFR_IMAGES_DIR}/${signed_cap_bin} \
        of=${PFR_IMAGES_DIR}/${output_bin}

    # deploy pfr images
    if [ -d ${PFR_DEPLOY_IMAGES_DIR} ]; then
        rm -rf ${PFR_DEPLOY_IMAGES_DIR}
    fi

    install -d ${PFR_DEPLOY_IMAGES_DIR}
    install -m 0644 ${PFR_IMAGES_DIR}/*.bin ${PFR_DEPLOY_IMAGES_DIR}/.
    install -m 0644 ${PFR_IMAGES_DIR}/*.bin_aligned ${PFR_DEPLOY_IMAGES_DIR}/.
    install -m 0644 ${PFR_IMAGES_DIR}/${output_bin} ${PFR_DEPLOY_IMAGES_DIR}/.
}

#make_sig_capsule() {
#    signature_files=""
#    for file in "$@"; do
#        openssl dgst -sha256 -sign ${SIGNING_KEY} -out "${file}.sig" $file
#        signature_files="${signature_files} ${file}.sig"
#    done
#}
#
#make_tar_capsule() {
#    files="$@"
#    tar -h -cvf signed_cap.tar ${files}
#}
#
#do_generate_pfr_capsule_tar[depends] += \
#    " obmc-phosphor-image:do_generate_static_tar"
#
#do_generate_pfr_capsule_tar() {
#    ln -sf ${S}/MANIFEST MANIFEST
#    ln -sf ${S}/publickey publickey
#    cp ${PFR_IMAGES_DIR}/bmc_signed_cap.bin image-img-stg
#    make_sig_capsule MANIFEST publickey image-img-stg
#    make_tar_capsule MANIFEST publickey image-img-stg ${signature_files}
#}

