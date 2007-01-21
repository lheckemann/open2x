#
# subset of YAFFS distribution. 
#   - bushi@mizi.com
#  
# YAFFS: Yet another FFS. A NAND-flash specific file system. 
#    http://www.aleph1.co.uk/armlinux/projects/yaffs/
#
# Copyright (C) 2002 Aleph One Ltd.
#   for Toby Churchill Ltd and Brightstar Engineering
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

# CONFIG_YAFFS_MTD_ENABLED
# This adds the yaffs file system support for working with a NAND mtd.
#
# CONFIG_YAFFS_USE_NANDECC
# This enables the ECC functions of the generic MTD-NAND driver.
# This will not work if you are using the old mtd.
# NB UseNANDECC does not work at present with yaffsram.
#
# CONFIG_YAFFS_ECC_WRONG_ORDER
# This makes yaffs_ecc.c use the same ecc byte order as
# Steven Hill's nand_ecc.c. If not set, then you get the
# same ecc byte order as SmartMedia.
#
# CONFIG_YAFFS_USE_GENERIC_RW
# Use generic_read/generic_write for reading/writing files. 
# This enables the use of the Linux file caching layer.
# If you disable this, then caching is disabled and file read/write is direct.
#
# CONFIG_YAFFS_DISABLE_CHUNK_ERASED_CHECK
# Enabling this turns off the test that chunks are erased in flash before 
#  writing to them.
# this is safe, since the write verification will fail.
# Suggest enabling the test (ie. keep the following line commented) during 
#  development to help debug things.
#
# CONFIG_YAFFS_DISABLE_WRITE_VERIFY
# I am severely reluctant to provide this config. 
# Disabling the verification is not a good thing to do since NAND writes 
#  can fail silently.
# Disabling the write verification will cause your teeth to rot, 
#  rats to eat your corn and give you split ends.
# You have been warned. ie. Don't uncomment the following line.
#
# CONFIG_YAFFS_SHORT_NAMES_IN_RAM
# If this config is set, then short names are stored with the yaffs_Object
# This costs an extra 16 bytes of RAM per object, but makes look-ups faster.
#
# CONFIG_YAFFS_USE_OLD_MTD
# Enable this to use the old MTD stuff that did not have yaffs support.
# You can use this to get around compilation problems, but the best
# thing to do is to upgrade your MTD support. You will get better speed.
#
# CONFIG_YAFFS_USE_HEADER_FILE_SIZE
# When the flash is scanned, two file sizes are constructed:
# * The size taken from the object header for the file.
# * The size figured out by scanning the data chunks.
# If this option is enabled, then the object header size is used, 
#  otherwise the scanned size is used.


YAFFS_CONFIGS := -DCONFIG_YAFFS_MTD_ENABLED \
		-DCONFIG_YAFFS_USE_GENERIC_RW \
		-DCONFIG_YAFFS_USE_NANDECC \
		-DCONFIG_SHORT_NAMES_IN_RAM

O_TARGET := yaffs.o

EXTRA_CFLAGS = $(YAFFS_CONFIGS)

obj-y := yaffs_fs.o yaffs_guts.o yaffs_mtdif.o yaffs_ecc.o

obj-m := $(O_TARGET)

include $(TOPDIR)/Rules.make
