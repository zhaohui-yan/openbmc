# Secure Boot Settings
MACHINE_FEATURES_append = " ast-secure"
ASPEED_SECURE_BOOT_TARGET = "ast2600"

# OpenBMC Settings
FLASH_UBOOT_SPL_IMAGE = "s_u-boot-spl"
FLASH_UBOOT_IMAGE = "s_u-boot"
FLASH_KERNEL_IMAGE = "s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}"
FLASH_KERNEL_IMAGE_df-obmc-ubi-fs = "s_fitImage-${MACHINE}.bin"

# ASPEED Initramfs Settings
ASPEED_IMAGE_UBOOT_SPL_IMAGE = "s_u-boot-spl"
ASPEED_IMAGE_UBOOT_IMAGE = "s_u-boot"
ASPEED_IMAGE_KERNEL_IMAGE = "s_fitImage-${INITRAMFS_IMAGE}-${MACHINE}-${MACHINE}"
