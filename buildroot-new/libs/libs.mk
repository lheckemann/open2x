#############################################################
#
# Open2x libraries
#
#############################################################

stat/o2x-libs_download:
	-@ echo "Downloading Open2x libraries..."
	-@ svn co https://open2x.svn.sourceforge.net/svnroot/open2x/trunk/libs-new/ $(DL_DIR)/o2x-libs
	-@ cp -f ./.config $(DL_DIR)/o2x-libs/.config
	-@ touch stat/o2x-libs_download

stat/o2x-libs_compile: stat/o2x-libs_download
	-@ echo "Compiling Open2x libraries..."
	-@ (cd $(DL_DIR)/o2x-libs; \
		./buildroot-build.sh \
	   )
	-@ touch stat/o2x-libs_compile

o2x-libs: stat/o2x-libs_compile
	-@ echo ""
	-@ echo "Open2x libraries installed"
	-@ echo ""
	-@ echo "*******************************"
	-@ echo ""

#############################################################
#
# Toplevel Makefile options
#
#############################################################
TARGETS+=o2x-libs
