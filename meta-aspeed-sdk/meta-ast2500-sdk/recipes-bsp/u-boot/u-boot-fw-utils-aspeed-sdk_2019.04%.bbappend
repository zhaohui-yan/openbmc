FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
FILESEXTRAPATHS:prepend:df-ast-img-sdk := "${THISDIR}/files/df-ast-img:"

ENV_CONFIG_FILE = "fw_env.config"
SRC_URI:append = " file://fw_env.config"
