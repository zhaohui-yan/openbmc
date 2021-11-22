FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# TODO: Remove this after next upgrade OpenBMC
SRCREV = "021d32cf907222cd72a5b9d2fe2e8159dd4bf083"

EXTRA_OEMESON:append= " \
    -Dredfish-dbus-log=enabled \
    -Dhttp-body-limit=128 \
    "

SRC_URI:append = " \
    file://0001-virtual-media-websocket-buffer-overflw.patch \
    file://0002-Support-websocket-control-frame-callback.patch \
    file://0003-Change-the-completionhandler-to-accept-Res.patch \
    "
