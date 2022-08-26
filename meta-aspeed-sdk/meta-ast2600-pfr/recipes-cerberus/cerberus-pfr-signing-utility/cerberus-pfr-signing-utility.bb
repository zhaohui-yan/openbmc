SUMMARY = "Cerberus PFR signing utility"
DESCRIPTION = "Cerberus PFR signing utility for manifest and recovery image creation"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=323954e5c90a1cc83ee74a3797f8d988"

SRC_URI = " git://gerrit.aspeed.com:29418/cerberus;protocol=ssh;branch=${BRANCH} "

PV = "1.0+git${SRCPV}"
SRCREV = "${AUTOREV}"
BRANCH = "aspeed-dev"

S = "${WORKDIR}/git"

inherit python3native setuptools3

DEPENDS:append = " ${PYTHON_PN}-pycryptodome-native "

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    # manifest tools
    install -d -m 0755 ${D}${datadir}/cerberus/manifest_tools
    install -m 0644 ${S}/tools/manifest_tools/*.py ${D}${datadir}/cerberus/manifest_tools/.
    install -m 0644 ${S}/tools/manifest_tools/*.xml ${D}${datadir}/cerberus/manifest_tools/.
    install -m 0644 ${S}/tools/manifest_tools/*.config ${D}${datadir}/cerberus/manifest_tools/.
    install -m 0644 ${S}/tools/manifest_tools/*.pem ${D}${datadir}/cerberus/manifest_tools/.
    # recovery tools
    install -d -m 0755 ${D}${datadir}/cerberus/recovery_tools
    install -m 0644 ${S}/tools/recovery_tools/*.py ${D}${datadir}/cerberus/recovery_tools/.
    install -m 0644 ${S}/tools/recovery_tools/*.xml ${D}${datadir}/cerberus/recovery_tools/.
    install -m 0644 ${S}/tools/recovery_tools/*.config ${D}${datadir}/cerberus/recovery_tools/.
    install -m 0644 ${S}/tools/recovery_tools/*.pem ${D}${datadir}/cerberus/recovery_tools/.
}

BBCLASSEXTEND = "native nativesdk"

FILES:${PN}:append = " ${datadir}/cerberus/* "
