#############################################################
#
# Kernel toolchain
#
#############################################################
KERNEL_FILE:=arm-open2x-linux-gcc-3.4.4-glibc-2.3.6_i686_linux.tar.bz2

stat/kernel_toolchain_download:
	-@ echo "Downloading kernel toolchain..."
	-@ $(WGET) --mirror -nd -P $(DL_DIR) http://www.open2x.org/open2x/toolchains/$(KERNEL_FILE).zip
	-@ mv $(DL_DIR)/$(KERNEL_FILE).zip $(DL_DIR)/$(KERNEL_FILE)
	-@ touch stat/kernel_toolchain_download

stat/kernel_toolchain_unpack: stat/kernel_toolchain_download
	-@ echo "Unpacking kernel toolchain..."
	-@ tar -xvjf $(DL_DIR)/$(KERNEL_FILE) -C /
	-@ touch stat/kernel_toolchain_unpack

kernel_toolchain: stat/kernel_toolchain_unpack
	-@ echo ""
	-@ echo "Kernel toolchain installed"
	-@ echo ""
	-@ echo "*******************************"
	-@ echo ""

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(O2X_TOOLCHAIN_KERNEL)),y)
TARGETS+=kernel_toolchain
endif
