# SUMMARY = "SE8 System Info D-Bus Interface"
# DESCRIPTION = "Provides D-Bus interface for SE8 system information"
# LICENSE = "Apache-2.0"
# LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

# SRC_URI = " \
#     file://xyz.openbmc_project.se8SystemInfo.Info.interface.yaml \
# "

# S = "${WORKDIR}"

# # inherit phosphor-dbus-interfaces
# INTERFACES_DIR = "${STAGING_DATADIR_NATIVE}/phosphor-dbus-interfaces"
# inherit native

# do_install() {
#     install -d ${D}${INTERFACES_DIR}
#     install -m 0644 ${S}/xyz.openbmc_project.se8SystemInfo.Info.interface.yaml \
#         ${D}${INTERFACES_DIR}
# }

# FILES:${PN} += "${INTERFACES_DIR}/xyz.openbmc_project.se8SystemInfo.Info.interface.yaml"
# SYSROOT_DIRS += "${interfaces_dir}"

LICENSE = " "
