project('ast-app', 'c')
thread_dep = dependency('threads')

# Video
executable('ast-video', ['video/ast-video.c', 'video/video.c'], install: true)
install_data('video/video.inf')

# i2c
executable('i2c-test', 'i2c-test/i2c-test.c', install: true, 
    dependencies: thread_dep)
executable('i2c-slave-mqueue', 'i2c-slave-mqueue/i2c-slave-mqueue.c', install: true)

# i3c
i3cincdir = include_directories('i3c-tools/include/i3c')
executable('i3c-test', 'i3c-test/i3c-test.c', install: true)
executable('i3ctransfer', 'i3c-tools/i3ctransfer.c', install: true, 
    include_directories: include_directories('i3c-tools/include/i3c'))

# eSPI
executable('mafs-test', 'espi_test/mafs-test.c', install: true)
executable('oob-test', 'espi_test/oob-test.c', install: true)
executable('perif-test', 'espi_test/perif-test.c', install: true)
executable('safs-test', 'espi_test/safs-test.c', install: true)
executable('vw-test', 'espi_test/vw-test.c', install: true)

# kcs/bt
executable('kcs-test', 'kcs-test/kcs-test.c', install: true, 
    dependencies: thread_dep)
executable('kcs_bt_test', 'kcs_bt_test/kcs_bt_test.c', install: true)

# mctp
executable('mctp', ['mctp/mctp.c', 'mctp/libmctp.c'], install: true, 
    include_directories: include_directories('mctp'), dependencies: thread_dep)
executable('mctp_rw', ['mctp_rw/mctp_rw.c', 'mctp/libmctp.c'], install: true, 
    include_directories: include_directories('mctp_rw'), dependencies: thread_dep)

# peci
executable('peci', 'peci/peci.c', install: true)
executable('peci_test', 'peci_test/peci_test.c', install: true)

# mem_utils
executable('md', ['mem_utils/md.c', 'mem_utils/mem_common.c'], install: true)
executable('mw', ['mem_utils/mw.c', 'mem_utils/mem_common.c'], install: true)
executable('spd', ['mem_utils/spd.c', 'mem_utils/mem_common.c'], install: true)
executable('spw', ['mem_utils/spw.c', 'mem_utils/mem_common.c'], install: true)

# svf
executable('svf', ['svf/main.c', 'svf/ast-jtag.c', 'svf/svf.c'], install: true)

# cpld
executable('cpld', ['cpld/main.c', 'cpld/lattice.c', 'cpld/ast-jtag.c'], install: true)

# random_mac
executable('random_mac', 'random_mac/random_mac.c', install: true)

# xdma
executable('xdma', 'xdma/xdma.c', install: true)

# tty-test
executable('tty-test', 'tty-test/tty-test.c', install: true, dependencies: thread_dep)
