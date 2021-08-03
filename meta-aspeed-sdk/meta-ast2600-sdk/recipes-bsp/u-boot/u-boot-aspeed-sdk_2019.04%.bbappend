FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
FILESEXTRAPATHS_prepend_ast-secure := "${THISDIR}/files/ast-secure:"

SRC_URI_append_ast-mmc = " \
    file://u-boot-env-ast2600.txt \
    file://s_u-boot-env-ast2600.txt \
    "
