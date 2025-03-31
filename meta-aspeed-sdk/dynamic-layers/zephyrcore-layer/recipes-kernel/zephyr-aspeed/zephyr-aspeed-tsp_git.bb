require recipes-kernel/zephyr-kernel/zephyr-image.inc
require zephyr-aspeed-src.inc

SUMMARY = "The Tertiary Service Processor (TSP) firmware"
PACKAGE_ARCH = "${MACHINE_ARCH}"

PROVIDES += "virtual/tsp"
PV = "1.0+git"

# Tag for v00.03.01
SRCREV_zephyr = "807ec0d7de758f7e11880ced45ea109ee8803d6a"
ZEPHYR_BRANCH = "aspeed-main-v3.7.0"

ZEPHYR_BOARD_TSP ??= "ast2700_evb/ast2700/tsp"
ZEPHYR_BOARD = "${ZEPHYR_BOARD_TSP}"

ZEPHYR_SRC_DIR ??= "${ZEPHYR_BASE}/samples/subsys/shell/shell_module"
