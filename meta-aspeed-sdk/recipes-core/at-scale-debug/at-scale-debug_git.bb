inherit obmc-phosphor-systemd

SUMMARY = "At Scale Debug Service"
DESCRIPTION = "At Scale Debug Service exposes remote JTAG target debug capabilities"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0d1c657b2ba1e8877940a8d1614ec560"


inherit cmake
DEPENDS = "sdbusplus openssl libpam libgpiod safec"

do_configure[depends] += "virtual/kernel:do_shared_workdir"

SRC_URI = "git://github.com/Intel-BMC/asd;protocol=https;branch=${BRANCH} \
           file://0001-Fix-build-error-in-kernel-v5.10.patch"

SRCREV = "f31661d92e80b3f097d37055f590595898cef6b6"
BRANCH = "master"

inherit useradd

USERADD_PACKAGES = "${PN}"

# add a special user asdbg
USERADD_PARAM_${PN} = "-u 9999 asdbg"

S = "${WORKDIR}/git"

SYSTEMD_SERVICE_${PN} += "com.intel.AtScaleDebug.service"

# Specify any options you want to pass to cmake using EXTRA_OECMAKE:
EXTRA_OECMAKE = "-DBUILD_UT=OFF"

CFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CFLAGS_append = " -I ${STAGING_KERNEL_DIR}/include/"
