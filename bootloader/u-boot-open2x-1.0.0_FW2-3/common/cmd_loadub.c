/*
 * (C) Copyright 2005
 * Kim Sungho, Dignsys Inc., shkim@dignsys.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * USB Boot support
 */
#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_LOADUB)

static char loadub_filename[128];
static int loadub_first_flag = 1;

extern int do_bootm (cmd_tbl_t *, int, int, char *[]);

static int loadub_common (cmd_tbl_t *, int , char *[]);

int do_loadub (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return loadub_common (cmdtp, argc, argv);
}

U_BOOT_CMD(
	loadub,	3,	1,	do_loadub,
	"loadub  - boot image via usb using usbdnw\n",
	"[loadAddress] [bootfilename]\n"
);

static int
loadub_common (cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	char *s;
	int   rcode = 0;
	int   size;

	if(loadub_first_flag) {
		/* pre-set filename */
		if ((s = getenv("bootfile")) != NULL) {
			copy_filename (loadub_filename, s, sizeof (loadub_filename));
			loadub_first_flag = 0;
		}
	}

	/* pre-set load_addr */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	switch (argc) {
	case 1:
		break;

	case 2:	/* only one arg - accept two forms:
		 * just load address, or just boot file name.
		 * The latter form must be written "filename" here.
		 */
		if (argv[1][0] == '"') {	/* just boot filename */
			copy_filename (loadub_filename, argv[1], sizeof(loadub_filename));
		} else {			/* load address	*/
			load_addr = simple_strtoul(argv[1], NULL, 16);
		}
		break;

	case 3:	load_addr = simple_strtoul(argv[1], NULL, 16);
		copy_filename (loadub_filename, argv[2], sizeof(loadub_filename));

		break;

	default: printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	printf("Filename: %s\n", loadub_filename);
	printf("Load address: 0x%x\n", load_addr);
	if ((size = usbd_recv(load_addr, loadub_filename)) < 0)
		return 1;

	/* done if no file was loaded (no errors though) */
	if (size == 0)
		return 0;

	/* flush cache */
	flush_cache(load_addr, size);

	/* Loading ok, check if we should attempt an auto-start */
	if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
		char *local_args[2];
		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n",
			load_addr);
		rcode = do_bootm (cmdtp, 0, 1, local_args);
	}

#ifdef CONFIG_AUTOSCRIPT
	if (((s = getenv("autoscript")) != NULL) && (strcmp(s,"yes") == 0)) {
		printf("Running autoscript at addr 0x%08lX ...\n", load_addr);
		rcode = autoscript (load_addr);
	}
#endif
	return rcode;
}

#endif	/* CFG_CMD_LOADUB */
