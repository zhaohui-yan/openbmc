DESCRIPTION = "Generate Secure Boot image for ASPEED BMC SoCs"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${ASPEEDSDKBASE}/LICENSE;md5=796e0355fcfe2a3345d663c4153c9e42"
PACKAGE_ARCH = "${MACHINE_ARCH}"

PR = "r0"

DEPENDS = " \
    socsec-native \
    aspeed-secure-config-native \
    virtual/bootloader \
    virtual/kernel \
    "

do_patch[noexec] = "1"
do_configure[noexec] = "1"
do_compile[noexec] = "1"
do_install[noexec] = "1"

inherit python3native deploy

# Image composition
UBOOT_SPL_IMAGE ?= "u-boot-spl.bin"
UBOOT_IMAGE ?= "u-boot.bin"
SUBOOT_SPL_IMAGE ?= "s_${UBOOT_SPL_IMAGE}"
SUBOOT_IMAGE ?= "s_${UBOOT_IMAGE}"

KERNEL_FIT_IMAGE ?= "fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}"
KERNEL_FIT_IMAGE_df-obmc-ubi-fs ?= "fitImage-${MACHINE}.bin"
SKERNEL_FIT_IMAGE ?= "s_${KERNEL_FIT_IMAGE}"

ASPEED_SECURE_BOOT_CONFIG_ROOT_DIR ?= "${STAGING_DATADIR_NATIVE}"

OUTPUT_IMAGE_DIR ?= "${S}/output"
SOURCE_IMAGE_DIR ?= "${S}/source"

print_otp_image() {
    if [ "${OTP_CONFIG}" != "" ]; then
        echo "Printing OTP Image ..."
        otptool \
        print \
        ${OUTPUT_IMAGE_DIR}/otp_image/otp-all.image

        if [ $? -ne 0 ]; then
            echo "Printed OTP image failed."
            exit 1
        fi
   fi
}

create_otp_image() {
    if [ "${OTP_CONFIG}" != "" ]; then
        echo "Generating OTP Image ..."
        otptool \
        make_otp_image \
        ${OTP_CONFIG} \
        --key_folder ${KEY_DIR} \
        --user_data_folder ${STAGING_DATADIR_NATIVE}/aspeed-secure-config/${ASPEED_SECURE_BOOT_TARGET}/security/data \
        --output_folder ${OUTPUT_IMAGE_DIR}/otp_image

        if [ $? -ne 0 ]; then
            echo "Generated OTP image failed."
            exit 1
        fi
   fi
}

create_secure_boot_image() {
    echo "Generating ROT Image ..."

    if [ "${ROT_ALGORITHM}" == "AES_GCM" ]; then
        socsec \
        make_secure_bl1_image \
        --algorithm ${ROT_ALGORITHM} \
        --bl1_image ${SOURCE_IMAGE_DIR}/${UBOOT_SPL_IMAGE} \
        --output ${OUTPUT_IMAGE_DIR}/${SUBOOT_SPL_IMAGE} \
        --gcm_aes_key ${ROT_SIGN_KEY} \
        --cot_algorithm ${COT_ALGORITHM} \
        --cot_verify_key ${COT_FIRST_VERIFY_KEY} \
        --stack_intersects_verification_region "false" \
        --signing_helper ${SIGNING_HELPER} \
        --signing_helper_with_files ${SIGNING_HELPER_WITH_FILES}
    else
        if [ "${AES_KEY_IN_OTP}" == "1" ]; then
            socsec \
            make_secure_bl1_image \
            --algorithm ${ROT_ALGORITHM} \
            --bl1_image ${SOURCE_IMAGE_DIR}/${UBOOT_SPL_IMAGE} \
            --output ${OUTPUT_IMAGE_DIR}/${SUBOOT_SPL_IMAGE} \
            --rsa_sign_key ${ROT_SIGN_KEY} \
            --rsa_key_order ${KEY_ORDER} \
            --cot_algorithm ${COT_ALGORITHM} \
            --cot_verify_key ${COT_FIRST_VERIFY_KEY} \
            --key_in_otp \
            --aes_key ${AES_KEY} \
            --rsa_aes ${RSA_AES_KEY} \
            --stack_intersects_verification_region "false" \
            --signing_helper ${SIGNING_HELPER} \
            --signing_helper_with_files ${SIGNING_HELPER_WITH_FILES}
        else
            socsec \
            make_secure_bl1_image \
            --algorithm ${ROT_ALGORITHM} \
            --bl1_image ${SOURCE_IMAGE_DIR}/${UBOOT_SPL_IMAGE} \
            --output ${OUTPUT_IMAGE_DIR}/${SUBOOT_SPL_IMAGE} \
            --rsa_sign_key ${ROT_SIGN_KEY} \
            --rsa_key_order ${KEY_ORDER} \
            --cot_algorithm ${COT_ALGORITHM} \
            --cot_verify_key ${COT_FIRST_VERIFY_KEY} \
            --aes_key ${AES_KEY} \
            --rsa_aes ${RSA_AES_KEY} \
            --stack_intersects_verification_region "false" \
            --signing_helper ${SIGNING_HELPER} \
            --signing_helper_with_files ${SIGNING_HELPER_WITH_FILES}
        fi
    fi

    if [ $? -ne 0 ]; then
        echo "Generated ROT image failed."
        exit 1
    fi

    if [ "${COT_ALGORITHM}" != "" ]; then \
        echo "Generating COT Image ..."
        socsec \
        make_sv_chain_image \
        --algorithm ${COT_ALGORITHM} \
        --image_relative_path ${SOURCE_IMAGE_DIR} \
        --cot_part ${COT_PARTITION} \
        --signing_helper ${SIGNING_HELPER} \
        --signing_helper_with_files ${SIGNING_HELPER_WITH_FILES}

        if [ $? -ne 0 ]; then
            echo "Generated COT image failed."
            exit 1
        fi

        if [ -f ${SOURCE_IMAGE_DIR}/${SUBOOT_IMAGE} ]; then
            mv ${SOURCE_IMAGE_DIR}/${SUBOOT_IMAGE} ${OUTPUT_IMAGE_DIR}
        fi

        if [ -f ${SOURCE_IMAGE_DIR}/${SKERNEL_FIT_IMAGE} ]; then
            mv ${SOURCE_IMAGE_DIR}/${SKERNEL_FIT_IMAGE} ${OUTPUT_IMAGE_DIR}
        fi
    fi

    if [ "${OTP_CONFIG}" != "" ]; then
        echo "Verifying OTP Image ..."
        socsec verify --sec_image ${OUTPUT_IMAGE_DIR}/${SUBOOT_SPL_IMAGE} --otp_image ${OUTPUT_IMAGE_DIR}/otp_image/otp-all.image

         if [ $? -ne 0 ]; then
            echo "Verified OTP image failed."
            exit 1
        fi
    fi
}

do_deploy () {
    unset ROOT_DIR
    unset UBOOT_IMAGE
    unset SUBOOT_IMAGE
    unset KERNEL_FIT_IMAGE
    unset KERNEL_SFIT_IMAGE

    export UBOOT_IMAGE="${UBOOT_IMAGE}"
    export SUBOOT_IMAGE="${SUBOOT_IMAGE}"
    export FIT_IMAGE="${KERNEL_FIT_IMAGE}"
    export SFIT_IMAGE="${SKERNEL_FIT_IMAGE}"
    export ROOT_DIR="${ASPEED_SECURE_BOOT_CONFIG_ROOT_DIR}"

    if [ -z ${SPL_BINARY} ]; then
        echo "To support ASPEED secure boot, u-boot should support spl."
        exit 1
    fi

    if [ -f ${ASPEED_SECURE_BOOT_CONFIG} ]; then
        source ${ASPEED_SECURE_BOOT_CONFIG}
    else
        bbwarn "User secure boot config not found!, ${ASPEED_SECURE_BOOT_CONFIG}"

        if [ ! -f ${STAGING_DATADIR_NATIVE}/aspeed-secure-config/${ASPEED_SECURE_BOOT_TARGET}/${ASPEED_SECURE_BOOT_CONFIG} ]; then
            echo "aspeed secure boot config not found!, ${STAGING_DATADIR_NATIVE}/aspeed-secure-config/${ASPEED_SECURE_BOOT_TARGET}/${ASPEED_SECURE_BOOT_CONFIG}"
            exit 1
        fi

        source ${STAGING_DATADIR_NATIVE}/aspeed-secure-config/${ASPEED_SECURE_BOOT_TARGET}/${ASPEED_SECURE_BOOT_CONFIG}
        bbwarn "Using an aspeed insecure config signing key!, ${STAGING_DATADIR_NATIVE}/aspeed-secure-config/${ASPEED_SECURE_BOOT_TARGET}/${ASPEED_SECURE_BOOT_CONFIG}"
    fi

    if [ -d ${SOURCE_IMAGE_DIR} ]; then
        rm -rf ${SOURCE_IMAGE_DIR}
    fi

    if [ -d ${OUTPUT_IMAGE_DIR} ]; then
        rm -rf ${OUTPUT_IMAGE_DIR}
    fi

    install -d ${SOURCE_IMAGE_DIR}
    install -d ${OUTPUT_IMAGE_DIR}

    # Install u-boot and u-boot-spl images into source directory.
    install -m 0644 ${DEPLOY_DIR_IMAGE}/${UBOOT_SPL_IMAGE} ${SOURCE_IMAGE_DIR}
    install -m 0644 ${DEPLOY_DIR_IMAGE}/${UBOOT_IMAGE} ${SOURCE_IMAGE_DIR}

    # Install kernel fit image into source directory.
    install -m 0644 ${DEPLOY_DIR_IMAGE}/${KERNEL_FIT_IMAGE} ${SOURCE_IMAGE_DIR}

    create_otp_image
    print_otp_image
    create_secure_boot_image

    # Deploy OTP image
    install -d ${DEPLOYDIR}
    install -d ${DEPLOYDIR}/otp_image
    install -m 0644 ${OUTPUT_IMAGE_DIR}/otp_image/* ${DEPLOYDIR}/otp_image/.

    # Deploy ROT and COT images
    install -m 0644 ${OUTPUT_IMAGE_DIR}/*.bin ${DEPLOYDIR}/.

    if [ -f ${OUTPUT_IMAGE_DIR}/${SKERNEL_FIT_IMAGE} ]; then
        install -m 0644 ${OUTPUT_IMAGE_DIR}/${SKERNEL_FIT_IMAGE} ${DEPLOYDIR}/.
    fi
}

do_deploy[depends] += " \
    virtual/kernel:do_deploy \
    virtual/bootloader:do_deploy \
    "

addtask deploy before do_build after do_compile
