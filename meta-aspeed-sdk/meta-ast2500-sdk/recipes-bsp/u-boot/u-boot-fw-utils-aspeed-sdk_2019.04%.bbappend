FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
FILESEXTRAPATHS:prepend_df-ast-img-sdk := "${THISDIR}/files/df-ast-img:"

ENV_CONFIG_FILE = "fw_env.config"
SRC_URI:append = " file://fw_env.config"

do_install:append () {
        install -m 644 ${WORKDIR}/${ENV_CONFIG_FILE} ${D}${sysconfdir}/fw_env.config
}

