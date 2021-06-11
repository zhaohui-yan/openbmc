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

EXTRA_IMAGE_FEATURES_append = " \
        nfs-client \
        "

### Workaround
inherit image_types_phosphor_aspeed
