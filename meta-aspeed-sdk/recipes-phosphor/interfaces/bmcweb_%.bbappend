FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

EXTRA_OEMESON_append_ast-mmc = " \
    -Dredfish-dbus-log=enabled \
    -Dhttp-body-limit=128 \
    "

SRC_URI_append = " file://0001-virtual-media-websocket-buffer-overflw.patch "
