IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

require recipes-core/images/core-image-minimal.bb

IMAGE_INSTALL_append = " \
        packagegroup-aspeed-apps \
        packagegroup-oss-apps \
        packagegroup-oss-libs \
        packagegroup-aspeed-crypto \
        ${@bb.utils.contains('MACHINE_FEATURES', 'ast-ssp', 'packagegroup-aspeed-ssp', '', d)} \
        "

EXTRA_IMAGE_FEATURES_append = " \
        nfs-client \
        "
