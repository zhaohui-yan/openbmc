FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
IMAGE_INSTALL_append = " \
        webui-vue \
        mctp \
        entity-manager \
        dbus-sensors \
        "

IMAGE_INSTALL_append = " \
        packagegroup-oss-apps \
        packagegroup-oss-obmc-apps \
        packagegroup-aspeed-apps \
        packagegroup-aspeed-crypto \
        packagegroup-aspeed-ssif \
        packagegroup-aspeed-obmc-inband \
        ${@bb.utils.contains('MACHINE_FEATURES', 'ast-ssp', 'packagegroup-aspeed-ssp', '', d)} \
        packagegroup-aspeed-mtdtest \
        "

# Only install in AST26xx series rofs as the free space of AST2500 rofs is not enough.
IMAGE_INSTALL_append_aspeed-g6 = " \
        packagegroup-aspeed-ktools \
       "

EXTRA_IMAGE_FEATURES_append = " \
        nfs-client \
        ${@bb.utils.contains('DISTRO_FEATURES', 'obmc-ubi-fs', 'read-only-rootfs-delayed-postinsts', '', d)} \
        ${@bb.utils.contains('DISTRO_FEATURES', 'phosphor-mmc', 'read-only-rootfs-delayed-postinsts', '', d)} \
        "

### Workaround
inherit image_types_phosphor_aspeed
