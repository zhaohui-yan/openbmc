FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGE_ARCH = "ts1000"

SRC_URI += " \
            file://aspeed-g5-se8.dtsi;subdir=git/arch/arm/boot/dts/aspeed \
            file://aspeed-ast2500-se8.dts;subdir=git/arch/arm/boot/dts/aspeed \
            file://aspeed-se8-flash-layout.dtsi;subdir=git/arch/arm/boot/dts/aspeed \
            file://aspeed_g5_se8_deconfig;subdir=git/arch/arm/configs \
           "
