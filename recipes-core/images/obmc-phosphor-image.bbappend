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
        "


### Workaround
inherit image_types_phosphor_aspeed
