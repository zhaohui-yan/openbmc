MACHINE_FEATURES_append = " ast-ssp"

# Do not load the "aspeed-ssp" module on boot.
KERNEL_MODULE_PROBECONF_append = " aspeed-ssp"
module_conf_aspeed-ssp = "blacklist aspeed-ssp"
