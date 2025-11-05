FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
IMAGE_INSTALL:append = " \
        libmctp \
        entity-manager \
        net-snmp-server \
        net-snmp-client \
        phosphor-snmp \
        sophgo-se8-extend-file \
        sophgo-se8-power-control \
        sophgo-se8-gpio-control \
        sophgo-eeprom-mac \
        smbios-mdr \
        "

IMAGE_INSTALL:append = " \
        packagegroup-oss-apps \
        packagegroup-oss-libs \
        packagegroup-oss-intel-pmci \
        packagegroup-aspeed-obmc-apps \
        packagegroup-aspeed-apps \
        packagegroup-aspeed-crypto \
        packagegroup-aspeed-ssif \
        packagegroup-aspeed-obmc-inband \
        packagegroup-aspeed-mtdtest \
        packagegroup-aspeed-usbtools \
        ${@bb.utils.contains('DISTRO_FEATURES', 'tpm', \
            bb.utils.contains('MACHINE_FEATURES', 'tpm2', 'packagegroup-security-tpm2', '', d), \
            '', d)} \
        packagegroup-aspeed-ktools \
        "

# uninstall packagegroup-oss-extra by default.
# IMAGE_INSTALL:append = " packagegroup-oss-extra "

# remove from AST25xx series rofs as the free space of AST25xx rofs is not enough.
IMAGE_INSTALL:remove:aspeed-g5 = " \
        packagegroup-aspeed-ktools \
        packagegroup-oss-extra \
        "

# packagegroup for ast2600
IMAGE_INSTALL:append:aspeed-g6 = " \
        ${@bb.utils.contains('MACHINE_FEATURES', 'ast-ssp', 'packagegroup-aspeed-coprocessor-ssp', '', d)} \
        "

# packagegroup for ast2700
IMAGE_INSTALL:append:aspeed-g7 = " \
        packagegroup-oss-extended \
        "

inherit extrausers

# password: root
SOPHGO_OPENBMC_PASSWORD = "'\$6\$UGMqyqdG\$uwiTSdLrKTW8i9c6BsZQQNCzQngW0tJM7jmaZnRgbT1Qi5EUra17TNmfyK0IUXwz.5BVx3Yk7evFW5sfpJgK20'"
EXTRA_USERS_PARAMS:pn-obmc-phosphor-image = " \
  usermod -p ${SOPHGO_OPENBMC_PASSWORD} root; \
  "

EXTRA_USERS_PARAMS:append = " \
useradd -e '' -ou 0 -d /home/root -G priv-admin,root,sudo,ipmi,web,redfish -p '\$6\$UGMqyqdG\$dkMIlCc6sbU.R45HNwnK3d2K2q5nULYq3xh69URx/I9GPvWhQonNdsvh8x15rYyKXoYxcB0Jhv0814XRVQnUJ0' admin; \
"




EXTRA_IMAGE_FEATURES:append = " \
        nfs-client \
        ${@bb.utils.contains('DISTRO_FEATURES', 'obmc-ubi-fs', 'read-only-rootfs-delayed-postinsts', '', d)} \
        ${@bb.utils.contains('DISTRO_FEATURES', 'phosphor-mmc', 'read-only-rootfs-delayed-postinsts', '', d)} \
        "

# Enable spi-nor-ecc.inc and unmask below to generate an image-rwfs with cleanmarker size set to 16.
#OVERLAY_MKFS_OPTS:spi-nor-ecc = " -c 16 -e 262144 --pad=${RWFS_SIZE} "

IMAGE_CLASSES:append:aspeed-g7 = " image_types_phosphor_aspeed_g7"

# We use direct-with-blksz.py to create WIC file for UFS.
# Do not generate scripts/lib/wic/plugins/imager/__pycache__/
IMAGE_CMD:wic:prepend:ast-ufs () {
        export PYTHONDONTWRITEBYTECODE="1"
}
