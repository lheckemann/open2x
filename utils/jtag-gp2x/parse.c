/* Open2x Project - GP2X JTAG Software
 * Copyright (C) 2006 The Open2x project
 *
 * Largely based on Jtag-Arm9 (http://jtag-arm9.sourceforge.net/)
 * Copyright (C) 2001 Simon Wood
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <ctype.h>

#include "jtag.h"
#include "debug.h"
#include "memory.h"
#include "regs.h"

/* unavoidable forward references */
static int parse_help(int argc, char **argv);
int parse_main(char * command_line);

char previous_command_line[128] = "";

static unsigned long hex_to_num(char * value)
{
	int temp;
	unsigned long number;
	unsigned base =  16;

	number = 0;

	for (temp=0; temp < strlen(value); temp ++) {
		if (value[temp] < 58 && value[temp] > 47)
			/* number */
			number = (number * base) + (value[temp] - 48);
		else if (value[temp] < 103 && value[temp] > 96)
			/* letter */
			number = (number * base) + (value[temp] - 97 + 10);
		else if (value[temp] == 43) {
			number = 0;
			base = 10;
		} else
			/* illegal character */
			return(number);
	}
	
	return(number);
}

static int parse_script(int argc, char **argv)
{
	FILE * script_file;
	char script_line[128];

	script_file = fopen(argv[0], "r");

	if (script_file == 0) {
		printf("Error opening script file : %s\n", argv[0]);
		return(1);
	}
	
	while (fgets(script_line, sizeof(script_line), script_file)) {
		if(parse_main(script_line)) {
			printf("Aborted - Error running:\n%s\n", script_line);
			break;
		}
	}

	fclose(script_file);

	/* rebuild previous_command_line */
	strcpy(previous_command_line, "script ");
	strcat(previous_command_line, argv[0]);

	return(0);
}

static int parse_echo(int argc, char **argv)
{
        int i;

        for (i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}

	printf("\n");

	return(0);
}

static int parse_pause(int argc, char **argv)
{
	unsigned long snooze = hex_to_num(argv[0]);

        printf("Pausing for 0x%8.8lX seconds\n", snooze);
        sleep(snooze);

	return(0);
}

static int parse_poke(int argc, char **argv)
{
        unsigned long address = hex_to_num(argv[0]);
	unsigned long data = hex_to_num(argv[1]);
	unsigned long length = (argc > 2 ? hex_to_num(argv[2]) : 1);

	memory_poke(address, data, length);

	return(0);
}

static int parse_peek(int argc, char **argv)
{
	unsigned long address = hex_to_num(argv[0]);
	unsigned long data = 0;

	memory_peek(address, &data);

	return(0);
}

static int parse_load(int argc, char **argv)
{
	char *file_name = argv[0];
	unsigned long address = hex_to_num(argv[1]);
        /* length in long words, or 0xFFFFFFFF for whole file */
        unsigned long length = argc > 2 ? hex_to_num(argv[2])/4 : 0xFFFFFFFF;

	FILE * data_file;
	unsigned long * data;
	unsigned long * data_start;

	unsigned long temp;
	unsigned long temp2;
	unsigned char data_byte;
	unsigned char data_ended = 0;

	data_file = fopen(file_name, "rb");

	if (data_file == 0) {
		printf("Error opening file : %s\n", file_name);
		return(1);
	}

	data_start = malloc(1024 * sizeof(unsigned long));

	if (data_start == NULL) {
		printf("Unable to allocate temporary storage\n");
		fclose(data_file);
		return(1);
	}

	data = data_start;

	while (length) {
		for (temp = 0; temp < 1024; temp++) {
			*data = 0;
	
			/* read in bytes convert to long words */
			for (temp2 = 0; temp2 < 4; temp2++) {
#if 1
				if (fread(&data_byte, sizeof(data_byte), 1, data_file) == 0) {
					data_ended = (unsigned char) EOF;
					break;
				}

#else
				data_byte = fgetc(data_file);

				if (data_byte == (unsigned char)EOF)
					break;
#endif				
				*data = (*data >> 8) + (data_byte << 24);
			}
			
			if (data_ended == (unsigned char)EOF) {
				length = temp + 1;
				break;
			}
	
			data++;
		}

		if (length < 1024 ) {
			memory_write(address, data_start, length);
			length = 0;
		} else {
			memory_write(address, data_start, temp);
			address = address + 4096;
			length = length - 1024;
			data = data_start;
		}
	}
		
	
	fclose(data_file);
	free(data_start);

	return(0);
}

static int parse_run(int argc, char **argv)
{
	arm_regs.pc = hex_to_num(argv[0]);
	debug_restart(argc, argv);

	return(0);
}

static int parse_regs(int argc, char **argv)
{
	int reg;
	unsigned long data;

        if (argc == 0) {
		regs_print();
		return(0);
	}

	/* ok trying to set a register value */
	reg = (int) hex_to_num(argv[0]);

	if (argc < 2) {
		printf("Usage: regs [<reg> <value>]\n");
		return(1);
	}
	data = hex_to_num(argv[1]);

	regs_set(reg, data);
#if 0
	regs_write();
	regs_read();
#endif
	regs_print();
	return(0);
}

static int parse_exit(int argc, char **argv)
{
    exit(0);
    return 0;
}

struct commands_block
{
    char *name;
    int	min_args;
    int	max_args;
    int (*function)(int argc, char **argv);
    char *syntax;
    char *description;
};

static struct commands_block commands[] =
{
    {
        /* Command, Min-Args, Max-Args, Function */
        "help", 0, 1, parse_help, 
        "[<command_name>]",     /* Syntax message */
        "See this help or help on an individual command" /* Help message */
    },
    {
        "script", 1, 1, parse_script,
        "<file_name>",
        "Source the file and interpret it"
    },
    {
        "echo", 0, 99, parse_echo,
        "[<characters>]*",
        "Echo the parameters to the user"
    },
    {
        "pause", 1, 1, parse_pause,
        "<seconds_to_sleep>",
        "Sleep for <n> seconds"
    },
    {
        "jtag", 0, 0, jtag_reset,
        "",
        "Initialise or reset the jtag bus"
    },
    {
        "idcode", 0, 0, jtag_idcode,
        "",
        "Print the ID code of the attached device"
    },
    {
        "quit", 0, 0, parse_exit,
        "",
        "Quit this program"
    },
    {
        "poll", 0, 0, debug_poll,
        "",
        "Poll the debug state"
    },
    {
        "halt", 0, 0, debug_halt,
        "",
        "Halt the CPU and put it into debug mode"
    },
    {
        "restart", 0, 0, debug_restart,
        "",
        "Take the CPU out of debug mode and start it running"
    },
    {
        "run", 1, 1, parse_run,
        "<address>",
        "Start the processor running from the given hex address"
    },
    {
        "poke", 2, 3, parse_poke,
        "<address> <data> [<length>]",
        "Poke the <address> with the <data>.  Optional length defaults to 1"
    },
    {
        "peek", 1, 1, parse_peek,
        "<address>",
        "Peek the supplied hex address"
    },
    {
        "load", 2, 3, parse_load,
        "<file_name> <address> [<length>]",
        "Load the file with name <file_name> at the hex <address>.  If <length> is supplied it will only load that many bytes"
    },
    {
        "regs", 0, 2, parse_regs,
        "[<reg> <value>]",
        ""
    },
    /* END */
    { 0, 0, 0, 0, 0, 0 }
};

int parse_main(char * command_line)
{
    int argc = 0;
    char **argv = 0;
    char *p;
    int rc = 1;                 /* return error by default */

    if (*command_line == '.')
        strcpy(command_line, previous_command_line);
    else 
        strcpy(previous_command_line, command_line);

    /* first parse the arguments out of the incoming string */
    p = command_line;
    /* skip any white space on the start */
    while (*p && isspace(*p))
        p++;
    while (*p)
    {
        char *end;
        int argsize;

        /* find the end of the string */
        end = p + 1;
        while (*end && !isspace(*end))
            end++;
        argsize = end - p;
        argv = realloc(argv, sizeof(argv[0]) * (argc+1));
        if (argv == 0) {
            fprintf(stderr, "No memory for arg parsing (1)\n");
            exit(EXIT_FAILURE);
	}
        argv[argc] = malloc(argsize+1);
        if (argv[argc] == 0) {
            fprintf(stderr, "No memory for arg parsing (2)\n");
            exit(EXIT_FAILURE);
	}
        memcpy(argv[argc], p, argsize);
        argv[argc][argsize] = 0; /* 0 terminate */
        argc++;
        p = end;
        /* skip any white space on the end*/
        while (*p && isspace(*p))
            p++;
    }

    if (argc == 0 || argv[0][0] == '#')
    {
        rc = 0;                 /* empty command or comment is OK */
    }
    else
    {
        char *command_name = argv[0];
        struct commands_block *command;

        for (command = &commands[0]; command->name; command++)
        {
            if (strcmp(command->name, command_name) == 0)
            {
                int args = argc - 1;
                if (args < command->min_args)
                {
                    printf("Too few args (%i) for command: %s %s\n",
                    	args, command->name, command->syntax);
                    break;
                }
                if (command->max_args >= 0 && args > command->max_args)
                {
                    printf("Too many args (%i) for command: %s %s\n",
                    	args, command->name, command->syntax);
                    break;
                }
                rc = command->function(argc-1, argv+1);	/* do the command! */
                break;
            }
        }
        if (!command->name)
            printf("Unknown command '%s'\n", argv[0]);
    }

    /* free the memory used for the command line */
    if (argv)
    {
        int i;
        for (i = 0; i < argc; i++)
        {
            if (argv[i])
                free(argv[i]);
        }
        free(argv);
    }

    return rc;
}

static int parse_help(int argc, char **argv)
{
    struct commands_block *command;
    for (command = &commands[0]; command->name; command++)
    {
        if (argc == 0 || strcmp(command->name, argv[0]) == 0)
        {
            printf("%s %s - %s\n",
                   command->name,
                   command->syntax,
                   command->description );
        }
    }
    return(0);
}
