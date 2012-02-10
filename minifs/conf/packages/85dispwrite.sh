
PACKAGES+=" dispwrite"
hset dispwrite url "none"
hset dispwrite dir "../../apps/dispwrite"

configure-dispwrite() {
	configure echo Done
}
compile-dispwrite() {
	compile make
	compile echo Done
}
install-dispwrite() {
	cp dispwrite "$STAGING_USR"/bin/dispwrite
	compile echo Done
}
deploy-dispwrite() {
	deploy cp "$STAGING_USR"/bin/dispwrite "$ROOTFS"/usr/bin/
}
