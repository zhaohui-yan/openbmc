FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
IMAGE_INSTALL_append = " \
        webui-vue \
        mctp \
        entity-manager \
        dbus-sensors \
        "

IMAGE_INSTALL_append = " \
        packagegroup-aspeed-apps \
        packagegroup-oss-apps \
        packagegroup-aspeed-crypto \
        ${@bb.utils.contains('MACHINE_FEATURES', 'ast-ssp', 'packagegroup-aspeed-ssp', '', d)} \
        "

### Workaround
inherit image_types_phosphor_aspeed
