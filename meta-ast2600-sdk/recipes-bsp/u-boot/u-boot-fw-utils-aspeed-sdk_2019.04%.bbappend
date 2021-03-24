FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

ENV_CONFIG_FILE = "fw_env_ast2600_nor.config"
SRC_URI_append = "file://fw_env_ast2600_nor.config"


do_install_append () {
	install -m 644 ${WORKDIR}/${ENV_CONFIG_FILE} ${D}${sysconfdir}/fw_env.config
}

