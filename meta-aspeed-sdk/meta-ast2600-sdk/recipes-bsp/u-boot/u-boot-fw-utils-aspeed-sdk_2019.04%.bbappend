FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
FILESEXTRAPATHS_prepend_df-ast-img-sdk := "${THISDIR}/files/df-ast-img:"

SRC_URI_append = " file://fw_env_ast2600_nor.config "
SRC_URI_append_ast-mmc = " file://fw_env_ast2600_mmc.config "

ENV_CONFIG_FILE = "fw_env_ast2600_nor.config"
ENV_CONFIG_FILE_ast-mmc = "fw_env_ast2600_mmc.config"

do_install_append () {
        install -m 644 ${WORKDIR}/${ENV_CONFIG_FILE} ${D}${sysconfdir}/fw_env.config
}

