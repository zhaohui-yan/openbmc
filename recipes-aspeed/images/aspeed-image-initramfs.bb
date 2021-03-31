IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

require recipes-core/images/core-image-minimal.bb

IMAGE_INSTALL_append = " \
        packagegroup-aspeed-apps \
        "
