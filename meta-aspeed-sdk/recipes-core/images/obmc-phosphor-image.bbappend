FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
IMAGE_INSTALL:append = " \
        webui-vue \
        libmctp \
        entity-manager \
        dbus-sensors \
        "

IMAGE_INSTALL:append = " \
        packagegroup-oss-apps \
        packagegroup-aspeed-obmc-apps \
        packagegroup-aspeed-apps \
        packagegroup-aspeed-crypto \
        packagegroup-aspeed-ssif \
        packagegroup-aspeed-obmc-inband \
        ${@bb.utils.contains('MACHINE_FEATURES', 'ast-ssp', 'packagegroup-aspeed-ssp', '', d)} \
        packagegroup-aspeed-mtdtest \
        "

# Only install in AST26xx series rofs as the free space of AST2500 rofs is not enough.
IMAGE_INSTALL:append_aspeed-g6 = " \
        packagegroup-aspeed-ktools \
       "

EXTRA_IMAGE_FEATURES:append = " \
        nfs-client \
        ${@bb.utils.contains('DISTRO_FEATURES', 'obmc-ubi-fs', 'read-only-rootfs-delayed-postinsts', '', d)} \
        ${@bb.utils.contains('DISTRO_FEATURES', 'phosphor-mmc', 'read-only-rootfs-delayed-postinsts', '', d)} \
        "

### Workaround
inherit image_types_phosphor_aspeed
