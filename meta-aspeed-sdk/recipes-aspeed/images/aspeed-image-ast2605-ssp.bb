DESCRIPTION = "Generate AST2605 SSP image for ASPEED BMC SoCs"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${ASPEEDSDKBASE}/LICENSE;md5=796e0355fcfe2a3345d663c4153c9e42"
PACKAGE_ARCH = "${MACHINE_ARCH}"

PR = "r0"

DEPENDS = " \
    rom-patcher-native \
    "

do_patch[noexec] = "1"
do_configure[noexec] = "1"
do_compile[noexec] = "1"
do_install[noexec] = "1"

inherit python3native deploy

create_ast2605_ssp_image() {
    echo "Generating AST2605 SSP Image ..."
    if [ ! -z ${AST2605_PATCHED_SSP_PATH} ]; then
        install ${AST2605_PATCHED_SSP_PATH} ${DEPLOY_DIR_IMAGE}/boot.bin
        exit 0
    fi

    if [ ! -z ${AST2605_SSP_PATH} ]; then
        rom-patcher ${AST2605_SSP_PATH}
    else
        rom-patcher ${DEPLOY_DIR_IMAGE}/zephyr.bin
    fi
    install ${S}/boot.bin ${DEPLOY_DIR_IMAGE}
}

do_deploy () {
    create_ast2605_ssp_image
}

addtask deploy before do_build after do_compile
