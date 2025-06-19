require aspeed-ssp-tsp.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGE_ARCH = "ts1000"

SRC_URI += "file://ast2500-se8.dts;subdir=git/arch/arm/dts \
						file://Makefile;subdir=git/arch/arm/dts \
          "
