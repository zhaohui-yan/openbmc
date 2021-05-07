# Create build environment
## Prerequisite
### Ubuntu 18.04
```
sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
     build-essential chrpath socat libsdl1.2-dev xterm
```

### Fedora
```
sudo yum install gawk make wget tar bzip2 gzip python unzip perl patch \
     diffutils diffstat git cpp gcc gcc-c++ glibc-devel texinfo chrpath \
     ccache perl-Data-Dumper perl-Text-ParseWords perl-Thread-Queue socat \
     findutils which SDL-devel xterm
```

Reference:
- [OpenBMC/README.md](https://github.com/openbmc/openbmc#1-prerequisite)
- [Yocto Quick Start](https://www.yoctoproject.org/docs/1.8/yocto-project-qs/yocto-project-qs.html#the-linux-distro)

## Target the machine
```
source setup <machine> [build_dir]
Target machine must be specified. Use one of:
ast2500-default
ast2600-default
ast2600-emmc
ast2600-emmc-secure
ast2600-secure-gcm
ast2600-secure-rsa2048-sha256
ast2600-secure-rsa2048-sha256-ncot
ast2600-secure-rsa2048-sha256-o1
ast2600-secure-rsa2048-sha256-o2-priv
ast2600-secure-rsa2048-sha256-o2-pub
ast2600-secure-rsa3072-sha384
ast2600-secure-rsa3072-sha384-o1
ast2600-secure-rsa3072-sha384-o2-priv
ast2600-secure-rsa3072-sha384-o2-pub
ast2600-secure-rsa4096-sha512
ast2600-secure-rsa4096-sha512-o1
ast2600-secure-rsa4096-sha512-o2-priv
ast2600-secure-rsa4096-sha512-o2-pub
ast2600-a2
ast2600-a2-emmc
ast2600-a2-emmc-secure
ast2600-a2-secure-gcm
ast2600-a2-secure-rsa2048-sha256
ast2600-a2-secure-rsa2048-sha256-ncot
ast2600-a2-secure-rsa2048-sha256-o1
ast2600-a2-secure-rsa2048-sha256-o2-priv
ast2600-a2-secure-rsa2048-sha256-o2-pub
ast2600-a2-secure-rsa3072-sha384
ast2600-a2-secure-rsa3072-sha384-o1
ast2600-a2-secure-rsa3072-sha384-o2-priv
ast2600-a2-secure-rsa3072-sha384-o2-pub
ast2600-a2-secure-rsa4096-sha512
ast2600-a2-secure-rsa4096-sha512-o1
ast2600-a2-secure-rsa4096-sha512-o2-priv
ast2600-a2-secure-rsa4096-sha512-o2-pub
ast2600-a1
ast2600-a1-secure-gcm
ast2600-a1-secure-rsa2048-sha256
ast2600-a1-secure-rsa2048-sha256-ncot
ast2600-a1-secure-rsa2048-sha256-o1
ast2600-a1-secure-rsa2048-sha256-o2-priv
ast2600-a1-secure-rsa2048-sha256-o2-pub
ast2600-a1-secure-rsa3072-sha384
ast2600-a1-secure-rsa3072-sha384-o1
ast2600-a1-secure-rsa3072-sha384-o2-priv
ast2600-a1-secure-rsa3072-sha384-o2-pub
ast2600-a1-secure-rsa4096-sha512
ast2600-a1-secure-rsa4096-sha512-o1
ast2600-a1-secure-rsa4096-sha512-o2-priv
ast2600-a1-secure-rsa4096-sha512-o2-pub
```

1. AST2600

```
source setup ast2600-default [build_dir]
```

2. AST2500

```
source setup ast2500-default [build_dir]
```

## Build OpenBMC firmware
```
bitbake obmc-phosphor-image
```

## Build SDK Image (all.bin)
The difference between ASPEED SDK image and OpenBMC firmware is that ASPEED SDK image only has necessary applications and test tools for ASPEED testing without OpenBMC phosphor packages and services.
Add `ast-img-sdk` DISTRO_FEATURES and `df-ast-img-sdk` DISTROOVERRIDES to apply the following change in `conf/local.conf`:

```
# ASPEED initramfs if build aspeed-image-sdk
- #require conf/distro/include/ast-img-sdk.inc
+ require conf/distro/include/ast-img-sdk.inc
```

Then trigger the build.

```
bitbake aspeed-image-sdk
```

It changes the following settings.
1. Change INITRAMFS_IMAGE to `aspeed-image-initramfs` in `conf/local.conf`.

```
INITRAMFS_IMAGE_df-ast-img-sdk = "aspeed-image-initramfs"
```

2. Change device tree from `aspeed-ast2600-obmc.dtb` to `aspeed-ast2600-evb.dtb` in meta-ast2600-sdk/conf/machine/${MACHINE}.conf file, e.g.,

```
# ASPEED ast2600 evb dtb file if build aspeed-image-sdk
KERNEL_DEVICETREE_df-ast-img-sdk = "aspeed-ast2600-evb.dtb"
KERNEL_DEVICETREE = "aspeed-ast2600-obmc.dtb"
```

3. Change u-boot default config from `evb-ast2600-obmc-emmc_defconfig` to `evb-ast2600-emmc_defconfig` in meta-ast2600-sdk/conf/machine/${MACHINE}.conf` file for boot from eMMC, e.g.,

```
# ASPEED ast2600 evb config file if build aspeed-image-sdk
UBOOT_MACHINE_df-ast-img-sdk = "evb-ast2600-emmc_defconfig"
UBOOT_MACHINE = "evb-ast2600-obmc-emmc_defconfig"
```

4. Remove `phosphor-mmc` DISTRO_FEATURES and DISTROOVERRIDES in `meta-ast2600-sdk/conf/machine/${MACHINE}.conf` file for boot from eMMC, e.g.,

```
# remove phosphor-mmc distro feature if build aspeed-image-sdk
require ${@bb.utils.contains('INITRAMFS_IMAGE', 'aspeed-image-initramfs', '', 'conf/distro/include/phosphor-mmc.inc', d)}
```

Then the image will be built according to the setting of `meta-aspeed-sdk/meta-ast2600-sdk/conf/machine/${MACHINE}.conf`.

# Output image
After you successfully built the image, the image file can be found in: `[build_dir]/tmp/work/deploy/images/${MACHINE}/`.

## OpenBMC firmware

### Boot from SPI image
- `image-bmc`: whole flash image
- `image-u-boot`: u-boot-spl.bin + u-boot.bin
- `image-kernel`: Linux Kernel FIT image
- `image-rofs`: read-only root file system

### Boot from SPI with secure boot image
- `image-bmc`: whole flash image
- `image-u-boot`: s_u-boot-spl.bin(RoT) + s_u-boot.bin (CoT1)
- `image-kernel`: Linux Kernel FIT Image the same as s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE (CoT2)
- `image-rofs`: read-only root file system
- `s_u-boot-spl`: u-boot-spl.bin processed with socsec tool signing for RoT image
- `s_u-boot`: u-boot.bin processed with socsec tool siging for CoT1 image
- `s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}`: fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE} with processed socsec tool signing for CoT2 image
- `otp_image`: OTP image

### Boot from eMMC image
- `emmc_image-u-boot`: u-boot-spl.bin + u-boot.bin processed with gen\_emmc\_image.py for boot partition
- `obmc-phosphor-image-${MACHINE}.wic.xz`: compressed emmc flash image for user data partition

### Boot from eMMC with secure boot image
- `s_emmc_image-u-boot`: s_u-boot-spl.bin(RoT) + s_u-boot.bin(CoT1) for boot partition
- `obmc-phosphor-image-${MACHINE}.wic.xz`: compressed emmc flash image for user data partition
- `s_u-boot_spl`: u-boot-spl.bin processed with socsec tool signing for RoT image
- `s_u-boot`: u-boot.bin processed with socsec tool siging for CoT1 image
- `s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}`: fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE} with processed socsec tool signing for CoT2 image
- `otp_image`: OTP image

## SDK Image

### Boot from SPI image
- `all.bin`: whole flash image

### Boot from eMMC image
- `emmc_u-boot-spl`: u-boot-spl.bin processed with gen\_emmc\_image.py for boot partition
- `all.bin`: image consists of u-boot and fitImage for user data partition

### Boot from eMMC with secure boot image
- `s_u-boot-spl`: u-boot-spl.bin processed with socsec tool signing for boot partition
- `all.bin`: image consists of s_u-boot and s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE} for user data partition
- `s_u-boot`: u-boot.bin processed with socsec tool siging for CoT1 image
- `s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}`: fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE} with processed socsec tool signing for CoT2 image
- `otp_image`: OTP image

## Recovery Image via UART
- `recovery_u-boot-spl` : u-boot-spl.bin processed with gen_uart_booting_image.sh for recovery image via UART
- `recovery_s_u-boot-spl` : s_u-boot-spl.bin processed with gen_uart_booting_image.sh for recovery image via UART with secure boot

