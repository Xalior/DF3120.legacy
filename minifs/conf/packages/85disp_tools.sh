
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

PACKAGES+=" dispinfo"
hset dispinfo url "none"
hset dispinfo dir "../../apps/dispinfo"

configure-dispinfo() {
	configure echo Done
}
compile-dispinfo() {
	compile make
	compile echo Done
}
install-dispinfo() {
	cp dispinfo "$STAGING_USR"/bin/dispinfo
	compile echo Done
}
deploy-dispinfo() {
	deploy cp "$STAGING_USR"/bin/dispinfo "$ROOTFS"/usr/bin/
}

PACKAGES+=" disptest"
hset disptest url "none"
hset disptest dir "../../apps/disptest"

configure-disptest() {
	configure echo Done
}
compile-disptest() {
	compile make
	compile echo Done
}
install-disptest() {
	cp disptest "$STAGING_USR"/bin/disptest
	compile echo Done
}
deploy-disptest() {
	deploy cp "$STAGING_USR"/bin/disptest "$ROOTFS"/usr/bin/
}

PACKAGES+=" disppic"
hset disppic url "none"
hset disppic dir "../../apps/disppic"
hset disppic depends "libpng"

configure-disppic() {
	configure echo Done
}
compile-disppic() {
	compile make
	compile echo Done
}
install-disppic() {
	cp disppic "$STAGING_USR"/bin/disppic
	compile echo Done
}
deploy-disppic() {
	deploy cp "$STAGING_USR"/bin/disppic "$ROOTFS"/usr/bin/
}
