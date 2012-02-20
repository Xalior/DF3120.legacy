#!/bin/bash

. "$CONF_BASE"/arch/armv4t.sh

MINIFS_BOARD_UBOOT=df3120

board_set_versions() {
	hset linux version "2.6.36.3"
	TARGET_FS_SQUASH=0
	TARGET_INITRD=0
	TARGET_SHARED=1
# Pick a size, any size.
#	Remember to allow for
#	overhead, this is your
#	partition size not you
#	card size. Take some off.
#   16384   :   16M
#   32768   :   32M
#   DEFAULT IS 62M - for crappy cards.
	TARGET_FS_EXT_SIZE=62000
#   63488   :   62M
#   64512   :   63M
#       size is in BYTES, btw :)
	hset uboot url "git!git://repo.or.cz/u-boot-openmoko/parrot-frames.git#uboot-cevin-git.tar.bz2"
}

board_prepare() {
	TARGET_PACKAGES+=" strace gdbserver picocom uboot"
	TARGET_PACKAGES+=" bluez sdlplasma"
	TARGET_PACKAGES+=" dispwrite"
	TARGET_PACKAGES+=" plftool"
	TARGET_PACKAGES+=" dropbear"
	hset dbus deploy false
}

cevin-deploy-linux-bare() {
	deploy-linux-bare
	cp "$BUILD"/kernel.ub "$ROOTFS"/linux
}

cevin-deploy-uboot() {
	# make sure the u-boot is aligned on 512 blocks, for mtd_debug
	deploy dd if=u-boot.bin of="$ROOTFS"/u-boot.bin bs=512 conv=sync
}
