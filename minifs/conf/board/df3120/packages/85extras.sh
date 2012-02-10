
PACKAGES+=" dispwrite"
hset dispwrite url "none"
hset dispwrite dir /../../apps/dispwrite

configure-dispwrite() {
	configure echo Done
}
compile-dispwrite() {
	compile make
	compile echo Done
}
install-dispwrite() {
	compile make
	log_install cp dispwrite "$STAGING_USR"/bin/dispwrite
}
deploy-dispwrite() {
	deploy cp "$STAGING_USR"/bin/dispwrite "$ROOTFS"/usr/bin/
}

PACKAGES+=" plftool"
hset plftool url "git!http://oomz.net/git/df3120/plftool.git#plftool-df3120-git.tar.bz2"
hset plftool depends "uboot"

configure-plftool() {
	configure echo Done
}
compile-plftool-local() {
	( 	export PATH="$BASE_PATH"
	unset CC CXX GCC LD CFLAGS CXXFLAGS CPPFLAGS LDFLAGS ACLOCAL ; 
	unset PKG_CONFIG_PATH PKG_CONFIG_LIBDIR LD_LIBRARY_PATH ;
	make
	)
}
compile-plftool() {
	compile compile-plftool-local
}
install-plftool() {
	log_install echo Done
}
deploy-plftool() {
	deploy ./pack_uboot.sh df3120 ../uboot/u-boot.bin ../parrotDF3120.plf
}
