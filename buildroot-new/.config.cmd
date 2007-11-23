deps_config := \
	libs/Config.in \
	toolchain/kernel/Config.in \
	toolchain/application/Config.in \
	toolchain/Config.in \
	Config.in

$(BR2_DEPENDS_DIR)/config/auto.conf: \
	$(deps_config)

$(deps_config): ;
