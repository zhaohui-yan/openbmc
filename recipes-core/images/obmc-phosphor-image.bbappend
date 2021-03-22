FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
IMAGE_INSTALL_append = " \
        webui-vue mctp \
        gperf \
        iperf3 \
        entity-manager \
        dbus-sensors \
        pciutils \
        ethtool \
        mmc-utils \
        memtester \
        "
