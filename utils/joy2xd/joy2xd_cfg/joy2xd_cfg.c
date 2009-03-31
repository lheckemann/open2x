/*	joy2xd_cfg.c  joy2xd configuration utility for Open2X. 
	This is a support program for joy2xd.  joy2xd is a daemon
	that allows USB gamepads to control the GP2X directly
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
#include	<SDL.h>
#include	<SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include	"SFont.h"	
#include <linux/joystick.h>
#include <signal.h>

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

// User can specify threshold between 1-128 to adjust the joystick's deadzone.
// It gets converted to a number between 1-32767
#define MIN_USER_THRESHOLD 1
#define MAX_USER_THRESHOLD	128
#define DEFAULT_USER_THRESHOLD	96
#define MIN_THRESHOLD	1
#define MAX_THRESHOLD	32767


const int menu_title_y = 52;
const int menu_line_y[4] = { 83, 93, 103, 113 };
const int menu_line_x = 90;
const int gp2x_abort_button = GP2X_BUTTON_X;
extern int errno;
unsigned long *memregs32;
unsigned short *memregs16;
FILE settings_file;
int memfd;
int joyfd;		// /dev/js0
int user_threshold = DEFAULT_USER_THRESHOLD;			// Joystick deadzone, 1-128
int axis_threshold = DEFAULT_USER_THRESHOLD << 8;	// How far does a joy axis need to 
																	//	be pushed beforewe accept it as input?
																	//		(This is 1-32767)
//global surfaces
SDL_Surface* screen;				// framebuffer
SDL_Surface* background;		// background containing test images, etc

//global fonts
SFont_Font *font_wh; //white

//global constants
const int screenw = 320;
const int screenh = 240;
const int screenbpp = 16;
const int screenflags = SDL_DOUBLEBUF;
 
char settings_filename[80] = "/etc/config/open2x_joy2xd.conf";
char joy_filename[80] = "/dev/input/o2x_js0";

// Each GP2X button has one entry for each of these two arrays:

//// type_map tells whether a button should be controlled by a
//// joystick axis event or a joystick button event.  
//// 	If an axis event, its entry will be JS_EVENT_BUTTON, 0x01
//// 	If a button event, its entry will be JS_EVENT_AXIS,  0x02
//// 	If it is not to be controlled at all, its entry will be 0
//int type_map[19] = {	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//							0, 0, 0, 0, 0, 0, 0, 0, 0 }
//
//// value_map is referred to when a button is being controlled
//// by a joystick axis.
//// 	The button's entry will be the minimum threshold value,
//// 	positive, or negative, for that axis to represent a 
//// 	GP2X button press.
//int value_map[19] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//							0, 0, 0, 0, 0, 0, 0, 0, 0 }


// axis_map links the USB gamepad's axes to the GP2X's 
// stick axes.  The value stored is 1 more than the 
// real axis number (i.e., axis 0 will be stored as
// positive and negative 1, axis 1 will be positive
// and negative 2, and so forth).  This is because
// we can't have positive and negative 0 ;)
//
// If axis 0 -/+ controls left to right 
// movement of the GP2X stick, these values will exist:
// axis_map[AXIS_LEFT] == -1		axis_map[AXIS_RIGHT] == 1
//
// And if axis 1 -/+ controls down to up movement of the
// GP2SX stick, these values will be assigned:
// axis_map[AXIS_DOWN] == -2		axis_map[AXIS_UP] == 2
// joystick axis event or a joystick button event.  
//#define AXIS_UP 		0
//#define AXIS_DOWN 	1
//#define AXIS_LEFT		2
//#define AXIS_RIGHT	3
//int axis_map[4] = {	0, 0, 0, 0 };



//// button_map holds the USB joy buttons numbers that
//// activate the GP2X's non-stick-related buttons. 
////	-1 means disabled
//#define BUTTON_A 			0
//#define BUTTON_B			1
//#define BUTTON_X			2
//#define BUTTON_Y			3
//#define BUTTON_L			4
//#define BUTTON_R			5
//#define BUTTON_SELECT	6
//#define BUTTON_START		7
//#define BUTTON_VOLUP		8
//#define BUTTON_VOLDOWN	9
//#define BUTTON_CLICK		10
//int button_map[11] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

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
void init_map_vars(void);
int write_config(void);
int read_config(const char *settings_filename);
void *try_mmap (void *start, size_t length, int prot, int flags, int fd, off_t offset);
int init_gpio (void);
int open_joy (char *joyfile);
void close_joy (void);
int get_joy_axis(int *axis, int *value, int abortbutton);
int get_joy_button(int *button, int abortbutton, int skipbutton);
void flush_joy_queue(void);
void close_phys (void);
int get_status (int number);
int wait_for_button (void);
int load_graphics(void);
void unload_graphics(void);
void shutdown(void);
void blit_bg(void);
void show_anim(void);
void show_joy_error(void);
int configure_sequence(void);
int menu(void);
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

	int i, x, y;

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
				printf("Before read_config, axis_threshold = %d\n", axis_threshold);
				y = atoi(param);
				if (y >= MIN_USER_THRESHOLD && y <= MAX_USER_THRESHOLD)
				{
					user_threshold = y;
					axis_threshold = y << 8;
					if (axis_threshold < MIN_THRESHOLD)
						axis_threshold = MIN_THRESHOLD;
					if (axis_threshold > MAX_THRESHOLD)
						axis_threshold = MAX_THRESHOLD;
					printf("Read in axis_threshold of %d\n", axis_threshold);
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

// initializes axis and button maps
void init_map_vars(void)
{
	int i;
	axis_map[AXIS_UP] = 0;
	axis_map[AXIS_LEFT] = 0;
	for (i = 0; i < BUTTON_MAP_SIZE ; i++)
		button_map[i] = -1;
}
	

//  Return 1 on success, 0 on error.
int write_config(void)
{
	int i;
	FILE *f;
	char buf[50];

	f = fopen(settings_filename, "w");
	if (f == NULL)
	{
	  printf("Error opening file for writing: %s\n", settings_filename);
	  return 0;
	}
	else
	{
	  printf("Writing settings to file: %s\n", settings_filename);
	}

	fprintf(f, "AXIS_UP=%d\n", axis_map[AXIS_UP]);
	fprintf(f, "AXIS_LEFT=%d\n", axis_map[AXIS_LEFT]);
	for (i=0; i < BUTTON_MAP_SIZE; i++)
	{
		fprintf(f, "BUTTON_%02d=%d\n", i, button_map[i]);
	}
	fprintf(f, "DEADZONE=%d\n", user_threshold);

	return (fclose(f) == 0);
}

void *try_mmap (void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	char *p;
	int aa;

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

// returns 1 on success, 0 on error
int init_gpio (void)
{
	memfd = open("/dev/mem", O_RDWR);
	if (memfd == -1)
	{
		printf ("Opening /dev/mem failed\n");
		return 0;
	}

	memregs32 = try_mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, 0xc0000000);
	if (memregs32 == (unsigned long *)0xFFFFFFFF) return 0;

	memregs16 = (unsigned short *)memregs32;

	return 1;
}

int open_joy (char *joyfilename)
{
	joyfd = open(joyfilename, O_RDONLY | O_NONBLOCK);
	return (joyfd != -1);
}

void close_joy (void)
{
	if (joyfd != -1)
		close(joyfd);
}

//This function will wait until either the user presses a joystick axis
//far enough in a direction to register above the axis_threshold or until
//the user presses the designated GP2X abort button.
//return 0 if got axis value (along with axis and value (as +1 or -1) in reference parameters)
//return 1 if user pressed gp2x abort button
int get_joy_axis(int *axis, int *value, int abortbutton)
{
	struct js_event ev;

	while (1)
	{
		if (get_status(abortbutton))
			return 1;						// user aborted
		if (read (joyfd, &ev, sizeof(struct js_event)) == sizeof(struct js_event))
		{
			if (ev.type == JS_EVENT_AXIS && abs(ev.value) > axis_threshold)
			{
				*axis = ev.number;
				*value = (ev.value > 0) ? 1 : -1;
				return 0;
			}
		}
	}
}

//This function will wait until either the user presses a joystick button
//or the user presses the designated GP2X abort button or skip buttons
//return 0 if we got a real button press
//return 1 if user pressed gp2x abort button
//return 2 if user pressed gp2x skip button
//return button number pressed in button reference parameter
int get_joy_button(int *button, int abortbutton, int skipbutton)
{
	struct js_event ev;

	while (1)
	{
		if (get_status(abortbutton))
			return 1;						// user aborted
		if (get_status(skipbutton))	
			return 2;						// user skipped
		if (read (joyfd, &ev, sizeof(struct js_event)) == sizeof(struct js_event))
		{
			if (ev.type == JS_EVENT_BUTTON && ev.value == 1 && ev.number < BUTTON_MAP_SIZE)
			{
				*button = ev.number;
				return 0;
			}
		}
	}
}

// Keep reading joy events until there aren't anymore (joystick centered and resting)
void flush_joy_queue(void)
{
	struct js_event ev;
	while (read (joyfd, &ev, sizeof(struct js_event)) > 0) {};
}

void close_phys (void)
{
	close (memfd);
}

// return status of a specific GP2X button, 1 if pressed, 0 if not
int get_status (int number)
{
	unsigned long mask = 0;
	switch (number) {
		case GP2X_BUTTON_UP:
			mask = 1;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask) 
				return 1;
			break;
		case GP2X_BUTTON_DOWN:
			mask = 1 << 4;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_LEFT:
			mask = 1 << 2;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_RIGHT:
			mask = 1 << 6;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask) 
				return 1;
			break;
		case GP2X_BUTTON_UPLEFT:
			mask = 1 << 1;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_UPRIGHT:
			mask = 1 << 7;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_DOWNLEFT:
			mask = 1 << 3;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_DOWNRIGHT:
			mask = 1 << 5;
			if (~(memregs16[GPIOMPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_VOLDOWN:
			mask = 1 << 6;
			if (~(memregs16[GPIODPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_VOLUP:
			mask = 1 << 7;
			if (~(memregs16[GPIODPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_CLICK:
			mask = 1 << 11;
			if (~(memregs16[GPIODPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_L:
			mask = 1 << 10;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_R:
			mask = 1 << 11;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_A:
			mask = 1 << 12;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_B:
			mask = 1 << 13;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_X:
			mask = 1 << 14;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_Y:
			mask = 1 << 15;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_SELECT:
			mask = 1 << 9;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		case GP2X_BUTTON_START:
			mask = 1 << 8;
			if (~(memregs16[GPIOCPINLVL >> 1]) & mask)
				return 1;
			break;
		default:
			// shouldn't happen
			printf("error in get_status()\n");
			return 0;
			break;
	}

	return 0;
}

// Wait infinitely for GP2X button to be pressed.
// Return value is button pressed.
int wait_for_button (void)
{
	int i;
	while (1) {
		for (i = 0; i <= 18; i++) {
			if (get_status(i)) {
				return i;
			}
		}
		usleep(20000); // sleep 20ms
	}
}

int load_graphics(void)
{
	SDL_Surface *tmp_sur, *tmp_sur2;

//	// animation that moves to show interlacing
//	tmp_sur = IMG_Load("img/animation.png");
//	if (!tmp_sur)
//	{
//		fprintf(stderr, "Fatal error loading: img/animation.png\n");
//		return 0;
//	}
//	animation = SDL_DisplayFormat(tmp_sur);
//	SDL_SetColorKey(animation, SDL_SRCCOLORKEY, SDL_MapRGB(screen->format, 255, 0, 255));
//	SDL_FreeSurface(tmp_sur);
	
	// Background image
	tmp_sur = IMG_Load("img/bg.png");
	if (!tmp_sur)
	{
		fprintf(stderr, "Fatal error loading: img/bg.png\n");
		return 0;
	}
	background = SDL_DisplayFormat(tmp_sur);
	SDL_FreeSurface(tmp_sur);
	
	//Font
	tmp_sur = IMG_Load("img/font_wh.png");
	if (!tmp_sur)
   {
        fprintf ( stderr,"Fatal error loading: img/font_wh.png\n" );
        return 0;
   }
   tmp_sur2 = SDL_DisplayFormat(tmp_sur);
   font_wh = SFont_InitFont(tmp_sur2);
	if (!font_wh)
   {
        fprintf ( stderr,"Fatal error intializing SFont on font_wh.png\n" );
        return 0;
   }
	SDL_FreeSurface(tmp_sur);
	

	return 1;
}

void unload_graphics(void)
{
	if (background)
		SDL_FreeSurface(background);

	if (font_wh) 
		SFont_FreeFont(font_wh);
}

void shutdown(void)
{
	unload_graphics();
	SDL_Quit();
	close_phys();
	close_joy();
}

void blit_bg(void)
{
	SDL_Rect dstrect;

	//blit background
	SDL_BlitSurface(background, NULL, screen, NULL);

	//blit menu border & BG
//	rectangleRGBA(screen, ((screenw >> 1) - 80), 1, ((screenw >> 1) + 80), 58, 
//			255, 255, 255, 255);		
	rectangleRGBA(screen, ((screenw >> 1) - 80), 51, ((screenw >> 1) + 80), 128, 
			0, 0, 0, 255);		
	dstrect.x = (screenw >> 1) - 80 + 1;
//	dstrect.y = 2;
	dstrect.y = 52;
	dstrect.w = ((screenw >> 1) + 80) - ((screenw >> 1) - 80) - 1;
//	dstrect.h = 56;
	dstrect.h = 76;
	SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));

	SFont_WriteCenter(screen, font_wh, menu_title_y, "Open2X Joy2xd Configuration Utility");		

}

// show animated menu window expanding on startup
void show_anim(void)
{
	SDL_Rect dstrect;

	int i;
	for (i = 0; i < 77; i+=4)
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);
		// animated menu border
		rectangleRGBA(screen, ((screenw >> 1) - i), 51, ((screenw >> 1) + i), 128, 
				0, 0, 0, 0);		
		dstrect.x = (screenw >> 1) - i + 1;
		dstrect.y = 52;
		dstrect.w = ((screenw >> 1) + i) - ((screenw >> 1) - i) - 2;
		dstrect.h = 75;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
		SDL_Flip(screen);
//		SDL_Delay(10);
	}
}


void show_joy_error(void)
{
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[0], "Error: No USB joysticks detected.");
	SFont_WriteCenter(screen, font_wh, menu_line_y[1], "Make sure USB Host and USB HID are");
	SFont_WriteCenter(screen, font_wh, menu_line_y[2], "enabled in the Open2X settings.");
	SFont_WriteCenter(screen, font_wh, menu_line_y[3], "Press any GP2X button to exit...");
	SDL_Flip(screen);
	wait_for_button();
}

// returns 1 if completed, 0 if cancelled
int configure_sequence(void)
{
#define CONFIG_INSTR_LINE1	0
#define CONFIG_INSTR_LINE2	1
#define CONFIG_INSTR_LINE3	2
#define CONFIG_INSTR_LINE4	3

	int rval,axis,button,value;
	int ok_delay = 1000;

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Move stick/pad up");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to cancel)");
	SDL_Flip(screen);
	rval = get_joy_axis(&axis, &value, gp2x_abort_button);
	switch (rval)
	{
		case 0:
			// User gave us USB joy axis
			axis_map[AXIS_UP] = axis + 1;		// always stored 1 more than real axis
														//	(since we can't have zero as positive and
														//	 negative)
			if (value < 0)
			{
				axis_map[AXIS_UP] *= -1;
			}
			break;
		default:
			// User aborted
			return 0;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok.");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok..");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);

	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Move stick/pad left");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to cancel)");
	SDL_Flip(screen);
	rval = get_joy_axis(&axis, &value, gp2x_abort_button);
	switch (rval)
	{
		case 0:
			// User gave us USB joy axis
			axis_map[AXIS_LEFT] = axis + 1;		// always stored 1 more than real axis
															//	(since we can't have zero as positive and
															//	 negative)
			if (value < 0)
			{
				axis_map[AXIS_LEFT] *= -1;
			}
			break;
		default:
			// User aborted
			return 0;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok.");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok..");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();
	
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X A button");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_A ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X B button");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_B ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X X button");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_X ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Y button");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_Y ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Left Trigger");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_L ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Right Trigger");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_R ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Volume -");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_VOLDOWN ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Volume +");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_VOLUP;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Select button");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_SELECT;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Start button");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_START ;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], "Press USB button corresponding to:");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "GP2X Stick-click");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], "(Press Start on GP2X to skip)");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE4], "(Press X on GP2X to go cancel)");
	SDL_Flip(screen);
	rval = get_joy_button(&button, gp2x_abort_button, GP2X_BUTTON_START);
	switch (rval)
	{
		case 0:
			// User gave us USB joy button
			button_map[button] = GP2X_BUTTON_CLICK;
			break;
		case 1:
			// User aborted
			return 0;
			break;
		default:
			// User skipped
			break;
	}
	blit_bg();
	SDL_Flip(screen);
	flush_joy_queue();
	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE2], "Ok");
	SDL_Flip(screen);
	SDL_Delay(ok_delay);
	flush_joy_queue();

	blit_bg();
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE1], 
			"Configuration complete.");
//	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], 
//			"Press a GP2X button to continue.");
	SFont_WriteCenter(screen, font_wh, menu_line_y[CONFIG_INSTR_LINE3], 
			"(pausing 4 seconds)");
	SDL_Flip(screen);
//	wait_for_button();
	SDL_Delay(4000);

	return 1;
}

int menu(void)
{
#define MENU_CONFIGURE_LINE		0
#define MENU_THRESHOLD_LINE		1	
#define MENU_QUIT_AND_SAVE_LINE	2
#define MENU_QUIT_LINE				3

	int i;
	const int menu_delay = 20;		// Menu loop delay in ms
	int quit = 0;
	int wants_to_save = 0;
	// User gets shown 1-128 and it gets converted internally to 1-32767
//	int user_threshold = axis_threshold >> 8; // = axis_threshold >> 15 << 7; 
//	if (user_threshold < MIN_USER_THRESHOLD)
//		user_threshold = MIN_USER_THRESHOLD;
//	if (user_threshold > MAX_USER_THRESHOLD)
//		user_threshold = MAX_USER_THRESHOLD;

	int last_input_time = 0;	// What was the ticks value when we last took a button input?

	int js_configured = 0;	// Turns to 1 when joystick configuration has been done

	SDL_Rect	srcrect, dstrect;

//	const int line_y[4] = { 13, 23, 33, 43 };
//	const int line_y[3] = { 23, 33, 43 };
//	const int line_x = 90;
	const int num_lines = 4;	// using 3 of 4 available lines

	int current_line = 0;			// What line of the menu to start on?

	char tmp_str[80];					// for drawing the threshold

//#define TIMING_LINE 			0
//#define CLOCKGEN_LINE 		1
//#define QUIT_AND_SAVE_LINE	2
//#define QUIT_LINE				3

	// show animated prettiness
	show_anim();


	while (!quit)
	{
		// menu loop

		js_configured = (axis_map[0] != 0);

		blit_bg();

		SFont_Write(screen, font_wh, menu_line_x, menu_line_y[MENU_CONFIGURE_LINE],
				"Reconfigure USB Joy 0");		
		
//		if (user_threshold == MIN_USER_THRESHOLD)
//			sprintf(tmp_str, "Adjust deadzone (def:%d)  %03d>", DEFAULT_USER_THRESHOLD,
//					user_threshold);
//		else if (user_threshold == MAX_USER_THRESHOLD)
//			sprintf(tmp_str, "Adjust deadzone (def:%d) <%03d", DEFAULT_USER_THRESHOLD,
//					user_threshold);
//		else
			sprintf(tmp_str, "Adjust deadzone (default %d): %d", DEFAULT_USER_THRESHOLD,
					user_threshold);
		SFont_Write(screen, font_wh, menu_line_x, menu_line_y[MENU_THRESHOLD_LINE],
				tmp_str);		

		if (js_configured)
		{
			SFont_Write(screen, font_wh, menu_line_x, menu_line_y[MENU_QUIT_AND_SAVE_LINE], 
					"Save and exit");		
		}
		SFont_Write(screen, font_wh, menu_line_x, menu_line_y[MENU_QUIT_LINE], 
				"Abandon changes and exit");		

		// draw cursor
		SFont_Write(screen, font_wh, menu_line_x - 4, menu_line_y[current_line], ">");
		SFont_Write(screen, font_wh, menu_line_x - 6, menu_line_y[current_line] - 1, "-");

		// update screen and wait a bit
		SDL_Flip(screen);

		if ((SDL_GetTicks() - last_input_time) > 150)
		{
			if (get_status(GP2X_BUTTON_A))
			{
				last_input_time = SDL_GetTicks();
				switch (current_line)
				{
					case MENU_THRESHOLD_LINE:
						user_threshold--;
						if (user_threshold < MIN_USER_THRESHOLD)
							user_threshold = MIN_USER_THRESHOLD;
						axis_threshold = user_threshold << 8;
						if (axis_threshold < MIN_THRESHOLD)
							axis_threshold = MIN_THRESHOLD;
						if (axis_threshold > MAX_THRESHOLD)
							axis_threshold = MAX_THRESHOLD;
						break;
					default:
						break;
				}
			}
			if (get_status(GP2X_BUTTON_B))
			{
				last_input_time = SDL_GetTicks();
				switch (current_line)
				{
					case MENU_CONFIGURE_LINE:
						if (!configure_sequence())
						{
							// user cancelled
							init_map_vars();
						}
						break;
					case MENU_THRESHOLD_LINE:
						user_threshold++;
						if (user_threshold > MAX_USER_THRESHOLD)
							user_threshold = MAX_USER_THRESHOLD;
						axis_threshold = user_threshold << 8;
						if (axis_threshold < MIN_THRESHOLD)
							axis_threshold = MIN_THRESHOLD;
						if (axis_threshold > MAX_THRESHOLD)
							axis_threshold = MAX_THRESHOLD;
						break;
					case MENU_QUIT_AND_SAVE_LINE:
						quit = 1;
						wants_to_save = 1;
						break;
					case MENU_QUIT_LINE:
						quit = 1;
						break;
					default:
						break;
				}
			}
			if (get_status(GP2X_BUTTON_L))
			{
				last_input_time = SDL_GetTicks();
				switch (current_line)
				{
					case MENU_THRESHOLD_LINE:
						user_threshold-=10;
						if (user_threshold < MIN_USER_THRESHOLD)
							user_threshold = MIN_USER_THRESHOLD;
						axis_threshold = user_threshold << 8;
						if (axis_threshold < MIN_THRESHOLD)
							axis_threshold = MIN_THRESHOLD;
						if (axis_threshold > MAX_THRESHOLD)
							axis_threshold = MAX_THRESHOLD;
						break;
					default:
						break;
				}
			}
			if (get_status(GP2X_BUTTON_R))
			{
				last_input_time = SDL_GetTicks();
				switch (current_line)
				{
					case MENU_THRESHOLD_LINE:
						user_threshold+=10;
						if (user_threshold > MAX_USER_THRESHOLD)
							user_threshold = MAX_USER_THRESHOLD;
						axis_threshold = user_threshold << 8;
						if (axis_threshold < MIN_THRESHOLD)
							axis_threshold = MIN_THRESHOLD;
						if (axis_threshold > MAX_THRESHOLD)
							axis_threshold = MAX_THRESHOLD;
						break;
					default:
						break;
				}
			}
			if (get_status(GP2X_BUTTON_UP))
			{
				last_input_time = SDL_GetTicks();
				current_line--;
				if (current_line == MENU_QUIT_AND_SAVE_LINE && !js_configured)
				{
					current_line--;
				}
				if (current_line < 0 )
				{
					current_line = 0;
				}
			}
			if (get_status(GP2X_BUTTON_DOWN))
			{
				last_input_time = SDL_GetTicks();
				current_line++;
				if (current_line == MENU_QUIT_AND_SAVE_LINE && !js_configured)
				{
					current_line++;
				}
				if (current_line >= num_lines)
				{
					current_line = num_lines-1;
				}
			}
			if (get_status(GP2X_BUTTON_LEFT))
			{
				last_input_time = SDL_GetTicks();
				switch (current_line)
				{
					case MENU_THRESHOLD_LINE:
						user_threshold--;
						if (user_threshold < MIN_USER_THRESHOLD)
							user_threshold = MIN_USER_THRESHOLD;
						axis_threshold = user_threshold << 8;
						if (axis_threshold < MIN_THRESHOLD)
							axis_threshold = MIN_THRESHOLD;
						if (axis_threshold > MAX_THRESHOLD)
							axis_threshold = MAX_THRESHOLD;
						break;
					default:
						break;
				}
			}
			if (get_status(GP2X_BUTTON_RIGHT))
			{
				last_input_time = SDL_GetTicks();
				switch (current_line)
				{
					case MENU_THRESHOLD_LINE:
						user_threshold++;
						if (user_threshold > MAX_USER_THRESHOLD)
							user_threshold = MAX_USER_THRESHOLD;
						axis_threshold = user_threshold << 8;
						if (axis_threshold < MIN_THRESHOLD)
							axis_threshold = MIN_THRESHOLD;
						if (axis_threshold > MAX_THRESHOLD)
							axis_threshold = MAX_THRESHOLD;
						break;
					default:
						break;
				}
			}
						
//			last_input_time = SDL_GetTicks();
		}
//		SDL_Delay(menu_delay);
		usleep(0);
	}

	return wants_to_save;
}

int initialize(void)
{
	if (!(screen = SDL_SetVideoMode(screenw, screenh, screenbpp, screenflags))) 
	{
		fprintf(stderr, "cannot open %dx%d display: %s\n",
			     screenw, screenh, SDL_GetError());
		return 0;
    } 
	
	SDL_ShowCursor(SDL_DISABLE);
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Cannot initialize SDL system: %s\n", SDL_GetError());
		return 0;
	}
	 
	if (!load_graphics()) {
		fprintf(stderr, "Error loading sprites/fonts.\n");
		shutdown();
		return 0;
	}

	if (!init_gpio()) {
		fprintf(stderr, "Error mapping GPIO registers!\n");
		shutdown();
		return 0;
	}

	if (!open_joy(joy_filename))
	{
		show_joy_error();
		shutdown();
		return 0;
	}

	init_map_vars();
	if (!read_config(settings_filename))
	{
		init_map_vars();
	}

	return 1;
}

int main(int argc, char *argv[])
{
	signal(SIGINT,&exit);
	signal(SIGTERM, &exit);

	if (argc > 1) {
		// We have been passed command-line parameters
		switch (argv[1][1]) {
			case 'h':
				// display help:
				printf("Joy2xd configuration utility written by Senor Quack v1.0\n");
				printf("Copyright (C) 2009 Daniel Silsby\n\n");
				printf("OPTIONS:\n");
				printf("-h\t\t:\tDisplay help and version info\n\n");
				printf("-sFILE\t\t:\tRead settings from this file\n");
				printf("\t\t\t(default is %s)\n", settings_filename);
				printf("-jJOYDEV\t:\tRead events from this joystick device file\n");
				printf("\t\t\t(default is %s)\n", joy_filename);
				return 0;
				break;
			case 's':
					if ((strlen(&argv[1][2]) < 79) && argv[1][2] != ' ' && argv[1][2] != '=')
					{
						strcpy(settings_filename, &argv[1][2]);
						printf("Settings file is %s\n", settings_filename);
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
						printf("Joy2xd_cfg reading events from %s\n", joy_filename);
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
				break;
		} // switch
	} 

	// Initialize graphics, load sprits/fonts etc, mmap registers
	if (!initialize())
	{
		fprintf(stderr, "Aborting because of initialization errors.\n");
		shutdown();
		return 1;
	}

	// RUN MENU LOOP
	if (menu())
	{
		// Returned 1, user wanted to save new values
		printf("User requested to save new configuration.\n");
		if (write_config())
		{
			printf("Successfully saved new configuration.\n");
		}
		else
		{
			printf("Problem occurred writing new configuration.\n");
		}

		sync();
	}

	shutdown();

	return 0;
}

