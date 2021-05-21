# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
#
# The following license files were not able to be identified and are
# represented as "Unknown" below, you will need to check them yourself:
#   COPYING.sunbmc
#
# NOTE: multiple licenses have been detected; they have been separated with &
# in the LICENSE value for now since it is a reasonable assumption that all
# of the licenses apply. If instead there is a choice between the multiple
# licenses then you should change the value to separate the licenses with |
# instead of &. If there is any doubt, check the accompanying documentation
# to determine which situation is applicable.
LICENSE = "GPLv3 & Unknown"
LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmi-fru;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ZRESEARCH;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.pstdout;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.bmc-watchdog;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmiconsole;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmimonitoring;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmiping;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmipower;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmi-dcmi;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.sunbmc;md5=c03f21cd76ff5caba6b890d1213cbfbb \
                    file://COPYING.ipmiseld;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.ipmidetect;md5=d32239bcb673463ab874e80d47fae504"

SRC_URI = "https://ftp.gnu.org/gnu/freeipmi/freeipmi-${PV}.tar.gz"
SRC_URI[md5sum] = "ad3b6ecbdc4d4b600b90133599dfd51d"
SRC_URI[sha1sum] = "80b4bfc628cf27d5681ba29bc02e11e7835214d5"
SRC_URI[sha256sum] = "cfa30179b44c582e73cf92c2ad0e54fe49f9fd87f7a0889be9dc2db5802e6aab"
SRC_URI[sha384sum] = "ef3981238d7354a8b43388059f2bef218718f9ac3960bcd2012114b11b056fb2d4071560f1bb268be8cc73c80a146a28"
SRC_URI[sha512sum] = "8a7be74bf003b8858c054bac24615f6fba0133e38e6f759ce81ed734a9ab107eb286c70554ad9663062c92eeccf342b80536aac0da74e4ede1ec51eedd497366"


# NOTE: the following prog dependencies are unknown, ignoring: cpp
# NOTE: the following library dependencies are unknown, ignoring: gcrypt gnugetopt argp
#       (this is based on recipes that have previously been built and packaged)

# NOTE: if this software is not capable of being built in a separate build directory
# from the source, you should replace autotools with autotools-brokensep in the
# inherit line
inherit autotools pkgconfig

# Specify any options you want to pass to the configure script using EXTRA_OECONF:
EXTRA_OECONF = " --without-encryption --without-random-device --with-dont-check-for-root "

