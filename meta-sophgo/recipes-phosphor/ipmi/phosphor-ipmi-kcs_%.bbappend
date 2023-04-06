# KCS1/2/3/4: LPC-KCS
# KCS6/7/8/9: PCIe-KCS

KCS_DEVICE = " \
    ipmi-kcs1 \
    ipmi-kcs2 \
    ipmi-kcs3 \
    ipmi-kcs4 \
    ipmi-kcs6 \
    ipmi-kcs7 \
    ipmi-kcs8 \
    ipmi-kcs9 \
"

SYSTEMD_SERVICE:${PN} = " \
    ${PN}@ipmi-kcs1.service \
    ${PN}@ipmi-kcs2.service \
    ${PN}@ipmi-kcs3.service \
    ${PN}@ipmi-kcs4.service \
    ${PN}@ipmi-kcs6.service \
    ${PN}@ipmi-kcs7.service \
    ${PN}@ipmi-kcs8.service \
    ${PN}@ipmi-kcs9.service \
"

