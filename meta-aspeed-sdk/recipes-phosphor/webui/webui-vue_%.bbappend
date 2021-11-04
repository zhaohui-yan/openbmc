FILESEXTRAPATHS:append := ":${THISDIR}/${PN}"

SRC_URI:append = " file://0001-poc-login-fix-waiting-in-login-page-after-login.patch"
