FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = " file://taskset.cfg "
SRC_URI_append = " file://mkfs.cfg "
SRC_URI_append = " file://dd.cfg "
SRC_URI_append = " file://mpstat.cfg "
