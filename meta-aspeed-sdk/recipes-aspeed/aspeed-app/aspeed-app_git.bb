# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# Unable to find any files that looked like license statements. Check the accompanying
# documentation and source headers and set LICENSE and LIC_FILES_CHKSUM accordingly.
#
# NOTE: LICENSE is being set to "CLOSED" to allow you to at least start building - if
# this is not accurate with respect to the licensing of the software being built (it
# will not be in most cases) you must specify the correct value before using this
# recipe for anything other than initial testing/development!
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

# S = "${WORKDIR}"

# No information for SRC_URI yet (only an external source tree was specified)
# http://192.168.10.30:7990/scm/bmc/ast_app.git

S = "${WORKDIR}/git"

SRC_URI = " git://192.168.10.13:29418/aspeed_app.git;protocol=ssh;branch=${BRANCH} \
            file://meson.build \
            file://video_ioctl.h \
"
PV = "1.0+git${SRCPV}"

# Build specific revision
# SRCREV = "84d288630f3f73dfb06e11d5e04e44b3899bacf4"

# Build latest revision
SRCREV = "4d675486db573d1cd31bc49249318f2ee064199b"
BRANCH = "master"
inherit meson

do_configure_prepend() {
  cp ${WORKDIR}/meson.build ${S}
  cp ${WORKDIR}/video_ioctl.h ${S}/ast_rvas/rvas_lib
}

FILES_${PN}_append = " /usr/share/* "
