/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define DEBUG

#include <common.h>

#if defined(CFG_ENV_IS_IN_NAND) /* Environment is in NAND */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>

#define CMD_SAVEENV

extern int nand_env_read(size_t start, size_t len, u_char *buf);
extern int nand_env_write(size_t start, size_t len, u_char *buf);
char * env_name_spec = "NAND";
env_t *env_ptr = (env_t *)CFG_NAND_ENV_BUFFER;

extern uchar default_environment[];
extern int default_environment_size;

uchar env_get_char_spec (int index)
{
	DECLARE_GLOBAL_DATA_PTR;
	return ( *((uchar *)(gd->env_addr + index)) );
}

int  env_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* read environment from NAND */
	memset (env_ptr,0,CFG_ENV_SIZE);
	nand_env_read(CFG_ENV_ADDR,ENV_SIZE,(u_char *)env_ptr);
	printf("Get Environment from NAND offset 0x%x ... \n",CFG_ENV_ADDR);

	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr  = (ulong)&(env_ptr->data);
		gd->env_valid = 1;
	} else {
		gd->env_addr  = (ulong)&default_environment[0];
		gd->env_valid = 0;
	}

	return (0);
}

#ifdef CMD_SAVEENV
int saveenv(void)
{
	//nand_erase_func(CFG_ENV_ADDR, ENV_SIZE);
	return nand_env_write(CFG_ENV_ADDR,ENV_SIZE,(u_char *)env_ptr);
}

#endif /* CMD_SAVEENV */

void env_relocate_spec (void)
{
	/* read environment from NAND */
	nand_env_read(CFG_ENV_ADDR,ENV_SIZE,(u_char *)env_ptr);
}

#endif /* CFG_ENV_IS_IN_NAND */
