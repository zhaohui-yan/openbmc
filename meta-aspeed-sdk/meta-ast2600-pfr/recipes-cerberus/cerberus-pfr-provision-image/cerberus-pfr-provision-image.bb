SUMMARY = "Cerberus PFR provision image"
DESCRIPTION = "Cerberus PFR provision image creation"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit python3native setuptools3

DEPENDS:append = " ${PYTHON_PN}-pycryptodome-native \
                   cerberus-pfr-signing-utility-native \
                 "

SRC_URI = " file://provision_tools;subdir=${S} "

do_patch[noexec] = "1"
do_configure[noexec] = "1"
do_compile[noexec] = "1"

PFR_PROVISION_TOOLS_DIR = "${STAGING_DIR_NATIVE}${datadir}/cerberus/provision_tools"

do_install() {
    rm -f ${PFR_PROVISION_TOOLS_DIR}/*.bin

    # install json and ini
    install -m 0644 ${S}/provision_tools/*.* ${PFR_PROVISION_TOOLS_DIR}/.

    cd ${PFR_PROVISION_TOOLS_DIR}
    python3 provisioning_image_generator.py provisioning_image_generator_rootkey.ini
    cd ${S}

    install -d ${D}${datadir}/cerberus
    install -m 0644 ${PFR_PROVISION_TOOLS_DIR}/*.bin ${D}${datadir}/cerberus/.
}

FILES:${PN}:append = " ${datadir}/cerberus/* "
