#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include "../../kexec.h"

#define OPT_APPEND     OPT_MAX+0
#define OPT_RAMDISK    OPT_MAX+1

#define COMMAND_LINESIZE      896

int zImage_arm_probe(const char *buf, off_t len)
{
	/* 
	 * Only zImage loading is supported. Do not check if
	 * the buffer is valid kernel image
	 */	
	return 0;
}
void zImage_arm_usage(void)
{
	printf("--command-line=STRING Pass a custom command line STRING to the kernel.\n"
	       "--initrd=FILENAME     Use the file FILENAME as a ramdisk.\n"
		);
}
int zImage_arm_load(int argc, char **argv, const char *buf, off_t len, 
	struct kexec_info *info)
{
	unsigned long base;
	unsigned int offset = 0x8000; /* 32k offset from memory start */

	void *krnl_buffer;
	char *rd_buffer;
	const char *command_line;
	const char *ramdisk;
	int command_line_len;
	off_t ramdisk_len;
	unsigned int ramdisk_origin;
	int opt;


	static const struct option options[] =
		{
			KEXEC_OPTIONS
			{"command-line",     1, 0, OPT_APPEND},
			{"initrd",           1, 0, OPT_RAMDISK},
			{0,                  0, 0, 0},
		};
	static const char short_options[] = KEXEC_OPT_STR "";

	ramdisk = NULL;
	command_line = NULL;
	ramdisk_len = 0;
	ramdisk_origin = 0;

	while ((opt = getopt_long(argc,argv,short_options,options,0)) != -1) {
		switch(opt) {
		case '?':
			usage();
			return -1;
			break;
		case OPT_APPEND:
			command_line = optarg;
			break;
		case OPT_RAMDISK:
			ramdisk = optarg;
			break;
		}
	}

	/* Process a given command_line: */
	if (command_line) {
		command_line_len = strlen(command_line) + 1; /* Remember the '\0' */
		if (command_line_len > COMMAND_LINESIZE) {
		        fprintf(stderr, "Command line too long.\n");
			return -1;
		}
	}

	base = locate_hole(info,len+offset,0,0,ULONG_MAX,INT_MAX);
	if (base == ULONG_MAX)
	{
		return -1;
	}
	base += offset;
	add_segment(info,buf,len,base,len);

	/* Load ramdisk if present */
	if (ramdisk) {
		rd_buffer = slurp_file(ramdisk, &ramdisk_len);
		if (rd_buffer == NULL) {
			fprintf(stderr, "Could not read ramdisk.\n");
			return -1;
		}
		add_segment(info, rd_buffer, ramdisk_len, 0xa1000000, ramdisk_len);
	}
	info->entry = (void*)base;
	return 0;
}
