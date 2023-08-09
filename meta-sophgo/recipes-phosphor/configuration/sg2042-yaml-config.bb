SUMMARY = "YAML configuration for sg2042"
PR = "r1"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"
FILESEXTRAPATHS:append := ":${THISDIR}/sg2042-yaml-config"
inherit allarch
SRC_URI = " \
    file://ipmi-fru.yaml \
    "
S = "${WORKDIR}"
do_install:append() {
    install -m 0644 -D ipmi-fru.yaml \
        ${D}${datadir}/${BPN}/ipmi-fru-read.yaml
}
FILES:${PN}-dev = " \
                   ${datadir}/${BPN}/ipmi-fru-read.yaml \
                  "
ALLOW_EMPTY:${PN} = "1"
