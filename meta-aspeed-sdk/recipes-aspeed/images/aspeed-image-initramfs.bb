IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

require recipes-core/images/core-image-minimal.bb

IMAGE_INSTALL_append = " \
        packagegroup-oss-apps \
        packagegroup-oss-libs \
        packagegroup-aspeed-apps \
        packagegroup-aspeed-crypto \
	packagegroup-aspeed-ssif \
        ${@bb.utils.contains('MACHINE_FEATURES', 'ast-ssp', 'packagegroup-aspeed-ssp', '', d)} \
        packagegroup-aspeed-mtdtest \
        packagegroup-aspeed-ktools \
        "

EXTRA_IMAGE_FEATURES_append = " \
        nfs-client \
        "
