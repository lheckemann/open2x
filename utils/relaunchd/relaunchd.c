/*	relaunchd.c  Menu relaunching daemon for Open2X. 
 	Copyright (C) 2008 Dan Silsby (Senor Quack)

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
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mmsp2_regs.h"

// GET_GMENU2X_RELAUNCH_NEEDED allows the gmenu2x relaunch daemon to see if the process killer
//    has indicated a relaunch of gmenu2x is needed.  Once the daemon relaunches GMenu2X, it will
//    issue the ioctl SET_GMENU2X_RELAUNCH_NEEDED with the argument 0 to reset this flag.
#define GP2X_GET_GMENU2X_RELAUNCH_NEEDED  92
#define GP2X_SET_GMENU2X_RELAUNCH_NEEDED  93

#define DEFAULT_DELAY	2	// 2 second delay
#define MIN_DELAY			1
#define MAX_DELAY			10
#define DEBUG 1

extern int errno;

int delay = DEFAULT_DELAY;
//char scriptname[256];
char *scriptname = "/usr/sbin/relaunchd_script";

int relaunch_needed(void)
{
	int gpiodev = open("/dev/GPIO",O_RDONLY);
	if (gpiodev == -1)
	{
#if DEBUG
		printf("Error opening /dev/GPIO during poll.\n");
#endif
		return 0;
	}

	int retval = (ioctl(gpiodev, GP2X_GET_GMENU2X_RELAUNCH_NEEDED, 0) == 1);
	close(gpiodev);
	return retval;
}

void reset_relaunch_flag(void)
{
	int new_flag = 0;
	int gpiodev = open("/dev/GPIO",O_WRONLY);
	if (gpiodev == -1)
	{
#if DEBUG
		printf("Error opening /dev/GPIO during reset.\n");
#endif
		return;
	}

	ioctl(gpiodev, GP2X_SET_GMENU2X_RELAUNCH_NEEDED, &new_flag);
	close(gpiodev);
}

void loop (void)
{
	struct timespec timereq, timerem;
	timereq.tv_sec = delay;
	timereq.tv_nsec = 0;

	while (1)
	{
		nanosleep(&timereq, &timerem);

		if (relaunch_needed())
		{
#if DEBUG
			printf("Relaunch needed, killing all gmenu2x processes..\n");
#endif
			system("killall gmenu2x");

#if DEBUG
			printf("Executing %s..\n", scriptname);
#endif
			if (system(scriptname) == -1)
			{
#if DEBUG
				printf("Error during relaunch\n");
#endif
			}

			reset_relaunch_flag();
		}
	}
}

void displayhelp (void)
{
	printf("Open2X menu relaunching daemon written by Senor Quack v1.0\n");
	printf("  Copyright (C) 2009 Daniel Silsby\n\n");
	printf("OPTIONS:\n");
	printf("-h\t\tDisplay help and version info\n");
//	printf("-sSCRIPTNAME\tRelaunch script (full pathname required)\n");
	printf("-dSECS\t\tSleep this many seconds inbetween polls. Valid range: %d..%d\n", 
			MIN_DELAY, MAX_DELAY);
	printf("\t\t\t(if not specified, default is %d seconds)\n", DEFAULT_DELAY);
}

int main(int argc, char *argv[])
{
	signal(SIGINT,&exit);
	signal(SIGTERM, &exit);
	signal(SIGHUP, SIG_IGN);

//	scriptname[0] = 0;
	int cur_arg = 1;
	while (cur_arg < argc)
	{
		// We have been passed command-line parameters
		switch (argv[cur_arg][1])
		{
			case 'h':
				displayhelp();
				return(0);
				break;
			case 'd':
				if (strlen(&argv[cur_arg][2]) <= 2)
				{
					delay = atoi(&argv[cur_arg][2]);
					if (delay < MIN_DELAY || delay > MAX_DELAY)
					{
						printf("ERROR: delay out of range\n\n");
						displayhelp();
						exit(1);
					}
				} else 
				{ 
					printf("ERROR: bad delay given\n\n");
					displayhelp();
					exit(1);
				}
				break;
//			case 's':
//				if (strlen(&argv[cur_arg][2]) < 250)
//				{
//					strcpy(scriptname, &argv[cur_arg][2]);
//					struct stat tmp;
//					if (stat(scriptname, &tmp))
//					{
//						printf("ERROR: file %s does not exist.\n\n", scriptname);
//						displayhelp();
//						exit(1);
//					}
//					strcat(scriptname, "&");	// run in background
//				} else 
//				{ 
//					printf("ERROR: script name too long\n\n");
//					displayhelp();
//					exit(1);
//				}
//				break;
			default:
				// invalid parameter
				printf("ERROR: invalid parameter passed\n\n");
				displayhelp();
				exit(1);
		} // switch
		cur_arg++;
	} // while (argc > 1) 

//	if (scriptname[0] == 0)
//	{
//		// No script specified
//		printf("ERROR: no relaunch script specified\n\n");
//		displayhelp();
//		exit(1);
//	}

	struct stat tmp;
	if (stat(scriptname, &tmp))
	{
		printf("ERROR: relaunching script %s is missing.\n\n", scriptname);
		exit(1);
	}

	// main loop
	loop();

	return 0;
}

