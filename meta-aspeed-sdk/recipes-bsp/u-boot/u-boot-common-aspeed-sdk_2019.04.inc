HOMEPAGE = "https://github.com/AspeedTech-BMC/u-boot"
SECTION = "bootloaders"
DEPENDS += "flex-native bison-native"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=30503fd321432fc713238f582193b78e"
PE = "1"

BRANCH="aspeed-dev-v2019.04"
SRC_URI = "git://192.168.10.13:29418/u-boot.git;protocol=ssh;branch=${BRANCH}"
SRC_URI:append:ast-abr = " file://0001-u-boot-disable-wdt2-for-abr.patch "
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"
B = "${WORKDIR}/build"
do_configure[cleandirs] = "${B}"

# We don't have 'u-boot-initial-env' target in our u-boot version
UBOOT_INITIAL_ENV=""

PV = "v2019.04+git${SRCPV}"

do_set_local_version() {
    rm -f ${S}/.scmversion
    rm -f ${B}/.scmversion
    is_tag="$(git -C ${S} name-rev --name-only --tags HEAD)"

    if [ $? -ne 0 ] ; then
        echo "not a git repository!!!"
    else
        if test "x${is_tag}" = "xundefined"; then
            version="$(git -C ${S} rev-parse --verify --short HEAD)"
            echo "-dirty-${version}" > ${S}/.scmversion
            echo "-dirty-${version}" > ${B}/.scmversion
        else
            version="$(git -C ${S} describe --tags --exact-match HEAD)"

            if echo "${version}" | grep -q "devtool"; then
                 version_ext="$(git -C ${S} rev-parse --verify --short HEAD)"
                 echo "-${version}-${version_ext}" > ${S}/.scmversion
                 echo "-${version}-${version_ext}" > ${B}/.scmversion
            else
                echo "-${version}" > ${S}/.scmversion
                echo "-${version}" > ${B}/.scmversion
            fi
        fi
    fi
}

addtask set_local_version before do_compile after do_configure
