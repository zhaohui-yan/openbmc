FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI:append = " file://settings.override.yml \
	                 file://time-sync-method.override.yml \
"
