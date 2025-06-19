SUMMARY = "Chassis Power Control service for Intel based platforms"
DESCRIPTION = "Chassis Power Control service for Intel based platforms"


FILESPATH := "${THISDIR}/project/:"


SRC_URI = " \
           file://config/power-config-host0.json \
           file://service_files/chassis-system-reset.service \
           file://service_files/chassis-system-reset.target \
           file://service_files/xyz.openbmc_project.Chassis.Control.Power@.service \
           file://src/power_control.cpp \
           file://src/power_control.hpp \
           file://MAINTAINERS \
           file://meson_options.txt \
           file://meson.build \
           file://OWNERS \
           file://README.md \
           "


S = "${WORKDIR}"

LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

inherit meson systemd pkgconfig
inherit obmc-phosphor-dbus-service

def get_service(d):
    if(d.getVar('OBMC_HOST_INSTANCES') == '0'):
      return "xyz.openbmc_project.Chassis.Control.Power@0.service"
    else:
      return " ".join(["xyz.openbmc_project.Chassis.Control.Power@{}.service".format(x) for x in d.getVar('OBMC_HOST_INSTANCES').split()])

SYSTEMD_SERVICE:${PN} = "${@get_service(d)}"

SYSTEMD_SERVICE:${PN} += "chassis-system-reset.service \
                         chassis-system-reset.target"

DEPENDS += " \
    boost \
    i2c-tools \
    libgpiod \
    nlohmann-json \
    sdbusplus \
    phosphor-logging \
  "
FILES:${PN}  += "${systemd_system_unitdir}/xyz.openbmc_project.Chassis.Control.Power@.service"

# /lib/systemd/system/xyz.openbmc_project.Chassis.Control.Power@.service
# /run/initramfs/ro/lib/systemd/system/xyz.openbmc_project.Chassis.Control.Power@.service
