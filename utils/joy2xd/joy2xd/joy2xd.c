/*	joy2xd.c  Joy2xd daemon for Open2X. 
	joy2xd is a daemon that allows USB gamepads to control the GP2X directly
	through special new ioctls issued to /dev/GPIO. 

 	Copyright (C) 2009 Dan Silsby (Senor Quack)

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
#include <linux/joystick.h>
#include <signal.h>
#include <time.h>


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
#define NUM_BUTTONS  (19) // total number of buttons we'll need to keep track of

// senquack - allows the gpiod daemon to set GP2X buttons to on/off even if the user isn't pressing
// 		anything on the GP2X itself.  For USB gamepad control of the GP2X.
// Argument to button_forcing is 1 if it is to be enabled, 0 if disabled:
#define GP2X_BUTTON_FORCING	70
// Argument to force_button ioctl is a 32-bit unsigned int.  The lower 16-bits should be set to 0x0001 
// 	if button is to be pressed down and 0x0000 if unpressed.  The upper 16-bits should be the button
// 	number to set, 0-18
#define GP2X_FORCE_BUTTON	71

// User can specify threshold between 1-128 to adjust the joystick's deadzone.
// It gets converted to a number between 1-32767
#define MIN_USER_THRESHOLD 1
#define MAX_USER_THRESHOLD	128
#define DEFAULT_USER_THRESHOLD	96
#define MIN_THRESHOLD 	1
#define MAX_THRESHOLD	32767

extern int errno;
//unsigned long *memregs32;
//unsigned short *memregs16;
FILE settings_file;
//int memfd;
int gpiofd;		// /dev/GPIO
int joyfd;		// /dev/js0
int axis_threshold = DEFAULT_USER_THRESHOLD << 8;	// How far does a joy axis need to 
																	//	be pushed beforewe accept it as input?
																	//		(This is 1-32767)
 
char settings_filename[80] = "/etc/config/open2x_joy2xd.conf";
char joy_filename[80] = "/dev/input/o2x_js0";
//char joy_filename[50] = "/dev/js0";
//char joy2_filename[50] = "/dev/js1";
char gpio_filename[] = "/dev/GPIO";

// axis_map links the USB gamepad's axes to the GP2X's 
// stick axes.  The value stored is 1 more than the 
// real axis number (i.e., axis 0 will be stored as
// positive and negative 1, axis 1 will be positive
// and negative 2, and so forth).  This is because
// we can't have positive and negative 0 ;)
//
// If axis 0 -/+ is left/right 
// and axis 1 -/+ is up/down movement
// movement of the USB joystick, these values will exist:
// axis_map[AXIS_LEFT] == -1		axis_map[AXIS_UP] == -2 
#define AXIS_UP 		0
#define AXIS_LEFT		1
int axis_map[2] = {	0, 0 };

// button_map holds the GP2X button numbers each joy button activates
//	-1 means disabled
#define BUTTON_MAP_SIZE 40
int button_map[40] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
								-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
								-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
								-1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

char *trim_string(char *buf);
int read_config(const char *settings_filename);
//void *try_mmap (void *start, size_t length, int prot, int flags, int fd, off_t offset);
int open_joy (void);
void close_joy (void);
int get_joy_axis(int *axis, int *value, int abortbutton);
int get_joy_button(int *button, int abortbutton, int skipbutton);
void flush_joy_queue(void);
int get_status (int number);
void shutdown(void);
int initialize(void);



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

//  Return 1 on success, 0 on error.  
int read_config(const char *settings_filename)
{
	FILE *f;
	char buf[8192];
	char *str, *param;

	int x, y;

	f = fopen(settings_filename, "r");
	if (f == NULL)
	{
	  printf("Error opening file: %s\n", settings_filename);
	  return 0;
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

			if ( strcmp(str, "AXIS_UP") == 0 )
			{ 
				if (strlen(param) <= 2 && atoi(param) != 0)
				{
					axis_map[AXIS_UP] = atoi(param);
				} else
				{
					printf("Error: ignoring bad parameter %s for key %s\n", param, str);
				}
			}
			else if ( strcmp(str, "AXIS_LEFT") == 0 )
			{ 
				if (strlen(param) <= 2 && atoi(param) != 0)
				{
					axis_map[AXIS_LEFT] = atoi(param);
				} else
				{
					printf("Error: ignoring bad parameter %s for key %s\n", param, str);
				}
			}
			else if ( strncmp(str, "BUTTON_", 7) == 0 )
			{ 
				x = atoi(str+7);
				y = atoi(param);
				if (x >= 0 && x < BUTTON_MAP_SIZE && y >= 0 && y < NUM_BUTTONS)
				{
					button_map[x] = y;
				} else if (y < -1)
				{
					printf("Error: ignoring bad key / parameter pair: %s=%s\n", str, param);
				}
			}
			else if ( strcmp(str, "DEADZONE") == 0 )
			{ 
				y = atoi(param);
				if (y >= MIN_USER_THRESHOLD && y <= MAX_USER_THRESHOLD)
				{
					axis_threshold = y << 8;
					if (axis_threshold < MIN_THRESHOLD)
						axis_threshold = MIN_THRESHOLD;
					if (axis_threshold > MAX_THRESHOLD)
						axis_threshold = MAX_THRESHOLD;
					printf("Saved axis_threshold: %d\n", axis_threshold);
				} else if (y < -1)
				{
					printf("Error: ignoring bad key / parameter pair: %s=%s\n", str, param);
				}
			}
			else
			{
				printf("Ignoring unknown setting: %s\n", str);
			}
	 }

	 if (axis_map[AXIS_UP] == 0 || axis_map[AXIS_LEFT] == 0)
	 {
		 printf("Fatal error: both Up and Left directional axes not specified\n");
		 return 0;
	 }

	 fclose(f);
	 return 1;
}

int open_gpio (char *filename)
{
	gpiofd = open(filename, O_RDWR);
	
	if (gpiofd != -1)
	{
		int x = 1;
		// Enable button forcing
		ioctl(gpiofd, GP2X_BUTTON_FORCING, &x);
		return 1;
	}

	return 0;
}

void close_gpio(void)
{
	if (gpiofd != -1)
	{
		int x = 0;
		// Disable button forcing
		ioctl(gpiofd, GP2X_BUTTON_FORCING, &x);
		close(gpiofd);
	}
}

int open_joy (void)
{
	int num_tries;
	struct timespec tv_req, tv_rem;

	tv_req.tv_sec = 1;
	tv_req.tv_nsec = 0;

	for (num_tries = 0; num_tries < 6; num_tries++)
	{
		joyfd = open(joy_filename, O_RDONLY);
		if (joyfd != -1)
			return 1;
		// wait a bit if we didn't open successfully, modules might be loading still
		nanosleep(&tv_req, &tv_rem);
	}

	return 0;
}

void close_joy (void)
{
	if (joyfd != -1)
		close(joyfd);
}

static inline void force_button(int button, int state)
{
	state |= (button << 16);
	ioctl(gpiofd, GP2X_FORCE_BUTTON, &state);
}
	
void read_loop(void)
{
	struct js_event ev;

	for (;;)
	{
		while (read (joyfd, &ev, sizeof(struct js_event)) == sizeof(struct js_event))
		{
			if (ev.type == JS_EVENT_BUTTON && ev.number < BUTTON_MAP_SIZE )
			{
				if (button_map[ev.number] != -1)
					force_button(button_map[ev.number], ev.value);
			}
			else if (ev.type == JS_EVENT_AXIS)
			{
				int up_pressed = 0; int down_pressed = 0;
				int left_pressed = 0; int right_pressed = 0;
				// is this equivalent to a down event? (greater than min. threshold?)
				int down_event = abs(ev.value) > axis_threshold;		
				if (abs(axis_map[AXIS_UP]) == (ev.number + 1))
				{
					// Movement along up/down axis
					if (axis_map[AXIS_UP] < 0 && (ev.value < 0))
					{
						// Movement towards up
						up_pressed = down_event;
						down_pressed = 0;
					} else
					{
						// Movement towards down
						down_pressed = down_event;
						up_pressed = 0;
					}
					force_button(GP2X_BUTTON_UP, up_pressed);
					force_button(GP2X_BUTTON_DOWN, down_pressed);
				} else if (abs(axis_map[AXIS_LEFT]) == (ev.number + 1))
				{
					// Movement along left/right axis
					if (axis_map[AXIS_LEFT] < 0 && (ev.value < 0))
					{
						// Movement towards left
						left_pressed = down_event;
						right_pressed = 0;
					} else
					{
						// Movement towards right
						right_pressed = down_event;
						left_pressed = 0;
					}
					force_button(GP2X_BUTTON_LEFT, left_pressed);
					force_button(GP2X_BUTTON_RIGHT, right_pressed);
				}
			}
		}
		usleep(0);	// Don't delay at all, just give up our timeslice
	}
}

void shutdown(void)
{
	close_gpio();
	close_joy();
}


int initialize(void)
{
	if (!read_config(settings_filename))
	{
		fprintf(stderr, "Joy2xd couldn't find configuration file %s\n", settings_filename);
		shutdown();
		return 0;
	}

	if (!open_joy())
	{
		fprintf(stderr, "Error opening joystick device %s\n", joy_filename);
		shutdown();
		return 0;
	}
	
	if (!open_gpio(gpio_filename))
	{
		fprintf(stderr, "Error opening device file %s\n", gpio_filename);
		shutdown();
		return 0;
	}
	
	return 1;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, &exit);
	signal(SIGTERM, &exit);
	signal(SIGHUP, SIG_IGN);

	if (argc > 1) {
		// We have been passed command-line parameters
		switch (argv[1][1]) {
			case 'h':
				// display help:
				printf("Joy2xd USB gamepad -> GP2X control daemon by Senor Quack v1.0\n");
				printf("Copyright (C) 2009 Daniel Silsby\n\n");
				printf("OPTIONS:\n");
				printf("-h\t\t:\tDisplay help and version info\n\n");
				printf("-d\t\t:\tJust disable GPIO emulation and terminate\n\n");
				printf("-sFILE\t\t:\tRead settings from this file\n");
				printf("\t\t\t(default is %s)\n", settings_filename);
				printf("-jJOYDEV\t:\tRead events from this joystick device file\n");
				printf("\t\t\t(default is %s)\n", joy_filename);

				return 0;
				break;
			case 'd':
				if (open_gpio(gpio_filename))
				{
					printf("Disabling GPIO emulation..\n");
					close_gpio();	// automatically disabled it
				}
				return 0;
				break;
			case 's':
					if ((strlen(&argv[1][2]) < 79) && argv[1][2] != ' ' && argv[1][2] != '=')
					{
						strcpy(settings_filename, &argv[1][2]);
						printf("Joy2xd reading config from %s\n", settings_filename);
					}
					else
					{
						printf("Error parsing filename after -s parameter.\n");
						return 1;
					}
				break;
			case 'j':
					if ((strlen(&argv[1][2]) < 79) && argv[1][2] != ' ' && argv[1][2] != '=')
					{
						strcpy(joy_filename, &argv[1][2]);
						printf("Joy2xd reading events from %s\n", joy_filename);
					}
					else
					{
						printf("Error parsing filename after -j parameter.\n");
						return 1;
					}
				break;
			default:
				// invalid parameter
				printf("Error: invalid parameter passed, see help by passing -h\n");
				shutdown();
				return 1;
		
		} // switch
	} 

	if (!initialize())
	{
		fprintf(stderr, "Aborting because of initialization errors.\n");
		shutdown();
		return 1;
	}

	read_loop();

	shutdown();

	return 0;
}

