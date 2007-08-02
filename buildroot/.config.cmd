deps_config := \
	target/gp2x/uboot/Config.in \
	target/tar/Config.in \
	target/ext2/Config.in \
	target/cramfs/Config.in \
	target/Config.in \
	package/samba/Config.in \
	package/utelnetd/Config.in \
	package/tinyx/Config.in \
	package/tftpd/Config.in \
	package/nano/Config.in \
	package/mkdosfs/Config.in \
	package/coreutils/Config.in \
	package/bash/Config.in \
	package/gmenu2x/Config.in \
	package/o2xlibs/Config.in \
	package/fakeroot/Config.in \
	package/busybox/Config.in \
	package/Config.in \
	toolchain/external-toolchain/Config.in \
	toolchain/Config.in \
	Config.in

include/config/auto.conf: \
	$(deps_config)

$(deps_config): ;
