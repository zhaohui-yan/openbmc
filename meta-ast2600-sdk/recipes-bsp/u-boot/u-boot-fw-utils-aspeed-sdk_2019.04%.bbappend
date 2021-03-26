FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = " file://fw_env_ast2600_nor.config "
SRC_URI_append_df-phosphor-mmc = " file://fw_env_ast2600_mmc.config "

ENV_CONFIG_FILE = "fw_env_ast2600_nor.config"
ENV_CONFIG_FILE_df-phosphor-mmc = "fw_env_ast2600_mmc.config"

do_install_append () {
	install -m 644 ${WORKDIR}/${ENV_CONFIG_FILE} ${D}${sysconfdir}/fw_env.config
}

