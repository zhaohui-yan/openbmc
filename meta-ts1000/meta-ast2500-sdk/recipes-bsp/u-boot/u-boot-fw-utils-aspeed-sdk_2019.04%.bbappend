FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

ENV_CONFIG_FILE = "fw_env.config"
SRC_URI:append = " file://fw_env.config"
