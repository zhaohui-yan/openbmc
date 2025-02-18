FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SYSTEMD_OVERRIDE:${PN} = "hotjoin.conf:pfr-mctp-i3c.service.d/hotjoin.conf"
SYSTEMD_OVERRIDE:${PN}:ast2700-a0 = "hotjoin_a0.conf:pfr-mctp-i3c.service.d/hotjoin_a0.conf"
