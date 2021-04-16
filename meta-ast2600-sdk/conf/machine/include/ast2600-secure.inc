# Secure Boot Settings
require conf/machine/include/ast-secure.inc

ASPEED_SECURE_BOOT_TARGET = "ast2600"

# OpenBMC Settings
FLASH_UBOOT_SPL_IMAGE = "s_u-boot-spl"
FLASH_UBOOT_IMAGE = "s_u-boot"
FLASH_KERNEL_IMAGE = "s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}"
FLASH_KERNEL_IMAGE_df-obmc-ubi-fs = "s_fitImage-${MACHINE}.bin"
# OpenBMC EMMC settings
IMAGE_BOOT_FILES = "s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE};s_fitImage"

# ASPEED Initramfs Settings
ASPEED_IMAGE_UBOOT_SPL_IMAGE = "s_u-boot-spl"
ASPEED_IMAGE_UBOOT_IMAGE = "s_u-boot"
ASPEED_IMAGE_KERNEL_IMAGE = "s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}"

# ASPEED EMMC Settings
ASPEED_EMMC_IMAGE_UBOOT_SPL_IMAGE = "s_u-boot-spl"
ASPEED_EMMC_IMAGE_UBOOT_IMAGE = "s_u-boot"
ASPEED_EMMC_IMAGE_MERGE_UBOOT_IMAGE = "s_emmc_image-u-boot"

