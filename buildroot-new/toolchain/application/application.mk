#############################################################
#
# Application toolchain
#
#############################################################
APPLICATION_FILE:=arm-open2x-linux-apps-gcc-4.1.1-glibc-2.3.6_i686_linux.tar.bz2

stat/application_toolchain_download:
	-@ echo "Downloading application toolchain..."
	-@ $(WGET) --mirror -nd -P $(DL_DIR) http://www.open2x.org/open2x/toolchains/$(APPLICATION_FILE).zip
	-@ mv $(DL_DIR)/$(APPLICATION_FILE).zip $(DL_DIR)/$(APPLICATION_FILE)
	-@ touch stat/application_toolchain_download

stat/application_toolchain_unpack: stat/application_toolchain_download
	-@ echo "Unpacking application toolchain..."
	-@ tar -xvjf $(DL_DIR)/$(APPLICATION_FILE) -C /
	-@ touch stat/application_toolchain_unpack

application_toolchain: stat/application_toolchain_unpack
ifeq ($(strip $(O2X_SET_ENV_VARS)),y)
	-@ echo export OPEN2X=\"/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/\" >> $(O2X_SET_ENV_VARS_FILE)
endif
	-@ echo ""
	-@ echo "Application toolchain installed"
	-@ echo ""
	-@ echo "*******************************"
	-@ echo ""

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(O2X_TOOLCHAIN_APPLICATION)),y)
TARGETS+=application_toolchain
endif
