LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

inherit pkgconfig meson

SRC_URI = " file://provision.c;subdir=${S} \
            file://provision.h;subdir=${S} \
            file://i2c_utils.c;subdir=${S} \
            file://i2c_utils.h;subdir=${S} \
            file://mailbox_enums.h;subdir=${S} \
            file://main.c;subdir=${S} \
            file://meson.build;subdir=${S} \
          "

DEPENDS = "openssl i2c-tools"

