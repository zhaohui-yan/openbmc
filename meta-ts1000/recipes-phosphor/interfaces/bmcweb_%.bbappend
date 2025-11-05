FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

EXTRA_OEMESON:append= " \
    -Dredfish-dbus-log=enabled \
    -Dhttp-body-limit=264 \
    "

SRC_URI:append = " \
    file://0001-bmcweb-fixes-virtual-media-buffer-overflow.patch \
    file://0002-Support-websocket-control-frame-callback.patch \
    file://0003-Modify-Content-Security-Policy-CSP-to-adapt-WebAssem.patch \
    file://0004-TS1000-remove-user-asdbg.patch \
    file://0005-add-se8SystemInfo.patch \
    file://0006-Add-backend-implementation-for-SNMP-configuration.patch \
    "




EXTRA_OEMESON:append = " \
    -Drest=enabled \
    "
