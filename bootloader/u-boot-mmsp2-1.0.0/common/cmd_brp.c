/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
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
 * nand flash boot code programming routine
 */

#include <common.h>

#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>

int do_brp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        int rc = 1;
        char buffer[100];

        if (argc  > 2)
        {
                printf ("Usage:\n%s\n", cmdtp->usage);
                return 1;
        }
        #if 1
        if (argc < 2)
        {
            printf("tftp 03000000 uImage\n");
            sprintf (buffer,"tftp 03000000 uImage ");
            rc = run_command(buffer, flag);

            if ( rc >= 0 )
            {
                printf("correct u-boot image!!!\n");
                run_command("nand erase 00040000 20000", flag);                
                sprintf (buffer,"nand write 03000000 00040000 %s ",getenv("filesize"));
                run_command(buffer, flag);
                printf("\nNow! - Power switch turn off & on~~~\n");
            }
        }

        if (argc == 2)
        {
            printf("tftp 03000000 %s\n",argv[1]);	
            sprintf (buffer,"tftp 03000000 %s ",argv[1]);
            rc = run_command(buffer, flag);

            if ( rc >= 0 )
            {
                printf("correct u-boot image!!!\n");
                run_command("nand erase 00040000 20000", flag);                
                sprintf (buffer,"nand write 03000000 00040000 %s ",getenv("filesize"));
                run_command(buffer, flag);
                printf("\nNow! - Power switch turn off & on~~~\n");
            }
        }
        #else /* write test address for the debug purpose, from 30000 with size 20000 */
        if (argc < 2)
        {
            printf("tftp 03000000 uImage\n");
            sprintf (buffer,"tftp 03000000 uImage ");
            rc = run_command(buffer, flag);

            if ( rc >= 0 )
            {
                printf("correct u-boot image!!!\n");
                run_command("nand erase 00030000 20000", flag);                
                sprintf (buffer,"nand write 03000000 00030000 %s ",getenv("filesize"));
                run_command(buffer, flag);
                printf("\nNow! - Power switch turn off & on~~~\n");
            }
        }

        if (argc == 2)
        {
            printf("tftp 03000000 %s\n",argv[1]);	
            sprintf (buffer,"tftp 03000000 %s ",argv[1]);
            rc = run_command(buffer, flag);

            if ( rc >= 0 )
            {
                printf("correct u-boot image!!!\n");
                run_command("nand erase 00030000 20000", flag);                
                sprintf (buffer,"nand write 03000000 00030000 %s ",getenv("filesize"));
                run_command(buffer, flag);
                printf("\nNow! - Power switch turn off & on~~~\n");
            }
        }
        #endif

        return 0;
}
/***************************************************/


U_BOOT_CMD(
	brp,	2,	1,	do_brp,
	"bootrom programming ex> brp - get default 'uImage'image to nand flash 0x00040000\n",
        "bootrom programming ex> brp [filename]"
);

