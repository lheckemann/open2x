/*	lcdadjuster.c  LCD adjustment utility for Open2X. 
 	Copyright (C) 2008 Dan Silsby (Senor Quack)
	 Portions of code based on :

	  cpuctrl.c for GP2X (CPU/LCD/RAM-Tuner Version 2.0)
    Copyright (C) 2006 god_at_hell 
    original CPU-Overclocker (c) by Hermes/PS2Reality 
	the gamma-routine was provided by theoddbot
	parts (c) Rlyehs Work

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <errno.h>

#define CLOCKGEN_UPLL 1
#define CLOCKGEN_FPLL 2

#define GPIOAPINLVL 0x1180
#define GPIOBPINLVL 0x1182
#define GPIOCPINLVL 0x1184
#define GPIODPINLVL 0x1186
#define GPIOEPINLVL 0x1188
#define GPIOFPINLVL 0x118A
#define GPIOGPINLVL 0x118C
#define GPIOHPINLVL 0x118E
#define GPIOIPINLVL 0x1190
#define GPIOJPINLVL 0x1192
#define GPIOKPINLVL 0x1194
#define GPIOLPINLVL 0x1196
#define GPIOMPINLVL 0x1198
#define GPIONPINLVL 0x119A
#define GPIOOPINLVL 0x119C

#define GPIOAOUT 0x1060
#define GPIOBOUT 0x1062
#define GPIOCOUT 0x1064
#define GPIODOUT 0x1066
#define GPIOEOUT 0x1068
#define GPIOFOUT 0x106A
#define GPIOGOUT 0x106C
#define GPIOHOUT 0x106E
#define GPIOIOUT 0x1070
#define GPIOJOUT 0x1072
#define GPIOKOUT 0x1074
#define GPIOLOUT 0x1076
#define GPIOMOUT 0x1078
#define GPIONOUT 0x107A
#define GPIOOOUT 0x107C

#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (15)
#define GP2X_BUTTON_Y               (14)
#define GP2X_BUTTON_L               (11)
#define GP2X_BUTTON_R               (10)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

extern int errno;
unsigned long *memregs32;
unsigned short *memregs16;
char settings_file[255] = "/etc/config/open2x_lcdsettings.conf";
int memfd;

struct lcd_timings
{
	unsigned int 	clockgen;
	signed int 		timing;	
};

struct lcd_timings new_timings;

// default is UPLL clockgen, timing of 1
const struct lcd_timings default_timings = { CLOCKGEN_UPLL, 1 };	

void set_add_FLCDCLK(int addclock)
{
	//Set LCD controller to use FPLL
	printf ("...set to FPLL-Clockgen...\r\n");
	printf ("set Timing-Prescaler = %i\r\n",addclock);
	memregs16[0x924>>1]= 0x5A00 + ((addclock)<<8); 
}

void set_add_ULCDCLK(int addclock)
{
	//Set LCD controller to use UPLL
	printf ("...set to UPLL-Clockgen...\r\n");
	printf ("set Timing-Prescaler = %i\r\n",addclock);
	memregs16[0x0924>>1] = 0x8900 + ((addclock)<<8);
}

unsigned get_LCDClk()
{
	if (memregs16[0x0924>>1] < 0x7A01) return((memregs16[0x0924>>1] - 0x5A00)>>8);
	else return((memregs16[0x0924>>1] - 0x8900)>>8);
}

char *trim_string(char *buf)
{
    int len;

    while (*buf == ' ') buf++;

    len = strlen(buf);

    while (len != 0 && buf[len - 1] == ' ')
    {
        len--;
        buf[len] = 0;
    }

    return buf;
}

void set_new_timings(struct lcd_timings timings)
{
	if (timings.clockgen == CLOCKGEN_UPLL)
	{
		if ((timings.timing >= -6) && (timings.timing <= 10))
		{
			printf("Setting clockgen to UPLL, timing: %d\n", timings.timing);
			set_add_ULCDCLK(timings.timing);
		}
	} 
	else if (timings.clockgen == CLOCKGEN_FPLL)
	{
		if ((timings.timing >= -20) && (timings.timing <= 36))
		{
			printf("Setting clockgen to FPLL, timing: %d\n", timings.timing);
			set_add_FLCDCLK(timings.timing);
		}
	}
}

//  Return 0 on success, -1 on error.  Read settings into *timings structure.
int read_saved_timings(char *settings_filename, struct lcd_timings *timings)
{
	FILE *f;
	char buf[8192];
	char *str, *param;

	f = fopen(settings_filename, "r");
	if (f == NULL)
	{
	  printf("Error opening file: %s\n", settings_filename);
	  return -1;
	}
	else
	{
	  printf("Loading settings from file: %s\n", settings_filename);
	}

    while (!feof(f))
    {
        // skip empty lines
        fscanf(f, "%8192[\n\r]", buf);

        // read line
        buf[0] = 0;
        fscanf(f, "%8192[^\n^\r]", buf);

        // trim line
        str = trim_string(buf);

        if (str[0] == 0) continue;
        if (str[0] == '#') continue;

        // find parameter (after '=')
        param = strchr(str, '=');

        if (param == NULL) continue;

        // split string into two strings
        *param = 0;
        param++;

        // trim them
        str = trim_string(str);
        param = trim_string(param);

        if ( strcasecmp(str, "clockgen") == 0 )
        {
            if ( strcasecmp(param, "fpll") == 0 ) 
            {
               timings->clockgen = CLOCKGEN_FPLL;
            }
            else if ( strcasecmp(param, "upll") == 0 ) 
            {
					timings->clockgen = CLOCKGEN_UPLL;
            }
				else
				{
					printf("Error parsing clockgen parameter in %s\n", settings_filename);
					printf("Setting parameter to default value.\n");
					timings->clockgen = default_timings.clockgen;
				}
        }
		  else if ( strcasecmp(str, "timing") == 0 )
		  {
			  if ((strlen(param) > 0) && (strlen(param) < 4))
			  {
				  timings->timing = atoi(param);
			  }
			  else
			  {
					printf("Error parsing timing parameter in %s\n", settings_filename);
					printf("Setting parameter to default value.\n");
					timings->timing = default_timings.timing;
			  }
		  }
		  else
		  {
			  printf("Ignoring unknown setting: %s\n", str);
		  }
	 }

	 // check range of values we got for timing
	 if (timings->clockgen == CLOCKGEN_UPLL)
	 {
		 if ((timings->timing < -6) || (timings->timing > 10))
		 {
			 printf("Timing parameter specified is not in the valid UPLL range of -6 to 10.\n");
			 printf("Setting all values to default.\n");
			 timings->clockgen = default_timings.clockgen;
			 timings->timing = default_timings.clockgen;
		 }
	 }
	 else if (timings->clockgen == CLOCKGEN_FPLL)
	 {
		 if ((timings->timing < -20) || (timings->timing > 36))
		 {
			 printf("Timing parameter specified is not in the valid FPLL range of -20 to 36.\n");
			 printf("Setting all values to default.\n");
			 timings->clockgen = default_timings.clockgen;
			 timings->timing = default_timings.clockgen;
		 }
	 }
	 else
	 {
		 printf("Clockgen not specified, setting all values to default.\n");
		 timings->clockgen = default_timings.clockgen;
		 timings->timing = default_timings.clockgen;
	 }

	 fclose(f);
	 return 0;
}


void *trymmap (void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	char *p;
	int aa;

	//printf ("mmap(%X, %X, %X, %X, %X, %X) ... ", (unsigned int)start, length, prot, flags, fd, (unsigned int)offset);
	p = mmap (start, length, prot, flags, fd, offset);
	if (p == (char *)0xFFFFFFFF)
	{
		aa = errno;
		printf ("mmap failed. errno = %d\n", aa);
	}
	else
	{
		//printf ("OK! (%X)\n", (unsigned int)p);
	}

	return p;
}

unsigned char initphys (void)
{
	memfd = open("/dev/mem", O_RDWR);
	if (memfd == -1)
	{
		printf ("Opening /dev/mem failed\n");
		return 0;
	}

	memregs32 = trymmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, 0xc0000000);
	if (memregs32 == (unsigned long *)0xFFFFFFFF) return 0;

	memregs16 = (unsigned short *)memregs32;

	return 1;
}

void closephys (void)
{
	close (memfd);
}

int main(int argc, char *argv[])
{
	// Initialize new timings to default timings for now
	new_timings.clockgen = default_timings.clockgen;
	new_timings.timing = default_timings.clockgen;

	if (!initphys()) {
		printf("Error mapping GPIO registers!\n");
		printf("Exiting..\n");
		return 1;
	}

	if (argc > 1) {
		// We have been passed command-line parameters
		switch (argv[1][1]) {
			case 'h':
				// display help:
				printf("CLI LCD timings adjustment utility written by Senor Quack v1.0\n");
				printf("Copyright (C) 2008 Daniel Silsby\n\n");
				printf("  Portions of code adapted from:\n");
				printf("  CPU/LCD/RAM-Tuner for GP2X Version 2.0\n");
				printf("  Copyright (C) 2006 god_at_hell \n\n");
				printf("OPTIONS:\n");
				printf("-h\t\t:\tDisplay help and version info\n\n");
				printf("-f FILENAME\t:\tRestore saved settings from this file\n\n");
				printf("\t(if not specified, default is %s\n", settings_file);
				break;
			case 'f':
				if (argc > 2)
				{
					// Good, we got passed a filename (presumably)
					if (strlen(argv[2]) < 254)
					{
						strcpy(settings_file, argv[2]);
					}
					else
					{
						printf("Error parsing filename after -f parameter.\n");
						return 1;
					}
				} 
				else
				{
					printf("Error parsing filename after -f parameter.\n");
					return 1;
				}

				if (read_saved_timings(settings_file, &new_timings) == 0)
				{
					//successfully read timings
					set_new_timings(new_timings);	
				}
				break;
			default:
				// invalid parameter
				printf("Error: invalid parameter passed, see help by passing -h\n");
		} // switch
	} // if (argc > 1) 
	else
	{
		printf("No filename specified on command line.\n");
		printf("Using default settings file: %s\n", settings_file);
		if (read_saved_timings(settings_file, &new_timings) == 0)
		{
			//successfully read timings
			set_new_timings(new_timings);	
		}
	}
			

	closephys();
	return 0;
}

