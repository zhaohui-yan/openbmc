do_generate_static:append() {
    bb.build.exec_func("do_generate_pfr_image", d)
}

mk_nor_image() {
    image_dst="$1"
    image_size_kb=$2
    dd if=/dev/zero bs=1k count=$image_size_kb \
        | tr '\000' '\377' > $image_dst
}

do_generate_pfr_image(){
    # Assemble the flash image
    mk_nor_image ${IMGDEPLOYDIR}/image-mtd ${PFR_IMAGE_SIZE}

    dd bs=1k conv=notrunc seek=0 \
        if=${IMGDEPLOYDIR}/${IMAGE_NAME}.static.mtd \
        of=${IMGDEPLOYDIR}/image-mtd
}

