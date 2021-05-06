FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = " file://0001-virtual-media-websocket-buffer-overflw.patch "
