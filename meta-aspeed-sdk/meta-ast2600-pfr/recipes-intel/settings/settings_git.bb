SUMMARY = "Settings"

SRC_URI = "git://github.com/Intel-BMC/settings.git;protocol=https;branch=master"
SRCREV = "85a8be9a3fb8ef4726899b28f10fb9afa6fa9e89"
PV = "0.1+git${SRCPV}"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=7becf906c8f8d03c237bad13bc3dac53"

SYSTEMD_SERVICE:${PN} = "xyz.openbmc_project.Settings.service"

DEPENDS = "boost \
           nlohmann-json \
           sdbusplus"

S = "${WORKDIR}/git"
inherit cmake systemd

EXTRA_OECMAKE = "-DYOCTO=1"

