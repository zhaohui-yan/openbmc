FILESEXTRAPATHS:append := ":${THISDIR}/${PN}"

SRC_URI:append = " file://0001-Update-to-vue-5.0.8.patch "
SRC_URI:append = " file://0002-Use-aspeed-s-novnc-fork.patch "
SRC_URI:append = " file://0003-revert-limit-to-on-chunk.patch "
SRC_URI:append = " file://0004-ts1000-V1.0.patch "
SRC_URI:append = " file://0005-Add-se8SystemInfo-to-web.patch "
SRC_URI:append = " file://0006-Add-SNMP-settings-page-in-web-UI.patch "

SRC_URI += "file://favicon.ico;subdir=git/ \
            file://zh-ZH.json;subdir=git/src/locales/ \
"
