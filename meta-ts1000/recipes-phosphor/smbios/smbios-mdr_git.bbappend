FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-TS1000-parse-smbios-table-without-MDR-header.patch"
