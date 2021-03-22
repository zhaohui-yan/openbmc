FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

DEPENDS = "jpeg"
RDEPENDS_${PN} = "jpeg"

SRC_URI_append = " http://s-tech.elsat.net.pl/fbv/fbv-1.0b.tar.gz "
SRC_URI_append = " file://meson.build "

SRC_URI[sha256sum] = "9b55b9dafd5eb01562060d860e267e309a1876e8ba5ce4d3303484b94129ab3c"

LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM_append = " file://COPYING;md5=130f9d9dddfebd2c6ff59165f066e41c "

inherit meson

do_configure_prepend() {
  cp ${WORKDIR}/meson.build ${S}
  cd ${S}
  ./configure --without-libungif --without-libpng
}

