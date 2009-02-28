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
#include	<SDL.h>
#include	<SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include	"SFont.h"	

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
#define NUMBUTTONS  (19) // total number of buttons we'll need to keep track of

extern int errno;
unsigned long *memregs32;
unsigned short *memregs16;
FILE settings_file;
int memfd;

//global surfaces
SDL_Surface* screen;				// framebuffer
SDL_Surface* background;		// background containing test images, etc
SDL_Surface* animation;			//	animation to show interlacing 

//global fonts
SFont_Font *font_wh; //white

//global joysticks
SDL_Joystick* joy;

//global constants
const int screenw = 320;
const int screenh = 240;
const int screenbpp = 16;
const int screenflags = SDL_DOUBLEBUF;
const int upll_max = 10;		// Min and max timings for each clockgen, upll and fpll
const int upll_min = -6;
const int fpll_max = 36;
const int fpll_min = -20;
 
//const char* settings_filename = "/etc/config/open2x_lcdsettings.conf";
char settings_filename[255] = "/etc/config/open2x_lcdsettings.conf";

struct lcd_timings
{
	unsigned int 	clockgen;
	signed int 		timing;	
};

struct lcd_timings original_timings, current_timings;

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

void set_new_timings(const struct lcd_timings timings)
{
	if (timings.clockgen == CLOCKGEN_UPLL)
	{
		if ((timings.timing >= upll_min) && (timings.timing <= upll_max))
		{
			printf("Setting clockgen to UPLL, timing: %d\n", timings.timing);
			set_add_ULCDCLK(timings.timing);
		}
	} 
	else if (timings.clockgen == CLOCKGEN_FPLL)
	{
		if ((timings.timing >= fpll_min) && (timings.timing <= fpll_max))
		{
			printf("Setting clockgen to FPLL, timing: %d\n", timings.timing);
			set_add_FLCDCLK(timings.timing);
		}
	}
}

//  Return 1 on success, 0 on error.  Write settings from timings structure.
int write_new_timings(const char *filename, struct lcd_timings timings)
{
	FILE *f;
	char buf[50];

	f = fopen(filename, "w");
	if (f == NULL)
	{
	  printf("Error opening file for writing: %s\n", filename);
	  return 0;
	}
	else
	{
	  printf("Writing settings to file: %s\n", filename);
	}

	if (timings.clockgen == CLOCKGEN_FPLL)
	{
		fprintf(f, "clockgen=fpll\n");
	}
	else
	{
		fprintf(f, "clockgen=upll\n");
	}

	sprintf(buf, "timing=%d\n", timings.timing);
	fprintf(f, buf);

	return (fclose(f) == 0);
}

//  Return 1 on success, 0 on error.  Read settings into *timings structure.
int read_saved_timings(const char *filename, struct lcd_timings *timings)
{
	FILE *f;
	char buf[8192];
	char *str, *param;

	f = fopen(filename, "r");
	if (f == NULL)
	{
	  printf("Error opening file: %s\n", filename);
	  return 0;
	}
	else
	{
	  printf("Loading settings from file: %s\n", filename);
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
					printf("Error parsing clockgen parameter in %s\n", filename);
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
					printf("Error parsing timing parameter in %s\n", filename);
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
		 if ((timings->timing < upll_min) || (timings->timing > upll_max))
		 {
			 printf("Timing parameter specified is not in the valid UPLL range of -6 to 10.\n");
			 printf("Setting all values to default.\n");
			 timings->clockgen = default_timings.clockgen;
			 timings->timing = default_timings.clockgen;
		 }
	 }
	 else if (timings->clockgen == CLOCKGEN_FPLL)
	 {
		 if ((timings->timing < fpll_min) || (timings->timing > fpll_max))
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
	 return 1;
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

int init_gpio (void)
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


int load_graphics(void)
{
	SDL_Surface *tmp_sur, *tmp_sur2;

	// animation that moves to show interlacing
	tmp_sur = IMG_Load("img/animation.png");
	if (!tmp_sur)
	{
		fprintf(stderr, "Fatal error loading: img/animation.png\n");
		return 0;
	}
	animation = SDL_DisplayFormat(tmp_sur);
	SDL_SetColorKey(animation, SDL_SRCCOLORKEY, SDL_MapRGB(screen->format, 255, 0, 255));
	SDL_FreeSurface(tmp_sur);
	
	// Background image with test gradients
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
        return 1;
   }
   tmp_sur2 = SDL_DisplayFormat(tmp_sur);
   font_wh = SFont_InitFont(tmp_sur2);
	if (!font_wh)
   {
        fprintf ( stderr,"Fatal error intializing SFont on font_wh.png\n" );
        return 1;
   }
   SDL_FreeSurface(tmp_sur);
	

	return 1;
}

void unload_graphics(void)
{
	SDL_FreeSurface(background);
	SDL_FreeSurface(animation);

	if (font_wh) 
	{
		SFont_FreeFont(font_wh);
	}
}

void shutdown(void)
{
	unload_graphics();
	SDL_Quit();
	closephys();
}


int menu(void)
{
	const int menu_delay = 10;		// Menu loop delay in ms
	int quit = 0;
	int wants_to_save = 0;

	int animation_y = 0;		// Where on the screen's y axis is the animation being blitted to?
	int animation_y_max = animation->h - 1;	// Max on y axis to blit to before going back to 0
	int animation_vel = 1;

	int last_input_time = 0;	// What was the ticks value when we last took a button input?

	SDL_Rect	srcrect, dstrect;

	const int line_y[4] = { 13, 23, 33, 43 };
	const int line_x = 90;
	const int num_lines = 4;

	int current_line = 0;			// What line of the menu to start on?

	char tmp_str[10];					// for drawing the timing on the menu

#define TIMING_LINE 			0
#define CLOCKGEN_LINE 		1
#define QUIT_AND_SAVE_LINE	2
#define QUIT_LINE				3

	int i;
	for (i = 0; i < 77; i+=4)
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);
		// animated menu border
		rectangleRGBA(screen, ((screenw >> 1) - i), 1, ((screenw >> 1) + i), 58, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - i + 1;
		dstrect.y = 2;
		dstrect.w = ((screenw >> 1) + i) - ((screenw >> 1) - i) - 2;
		dstrect.h = 55;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
		SDL_Flip(screen);
//		SDL_Delay(10);
	}

	while (!quit)
	{
		// menu loop

		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);

		// blit background color over which animation will be drawn
		dstrect.x = 0;
		dstrect.y = 0;
		dstrect.w = screenw;
		dstrect.h =	animation->h; 
//		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
//		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 150, 150, 150));
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 0, 0, 0));

		// blit animation to show interlacing
		srcrect.x = 0;
		srcrect.y = 0;
		srcrect.w = animation->w;
		srcrect.h = animation->h - animation_y;

		dstrect.x = 0;
		dstrect.y = animation_y;
		dstrect.w = srcrect.w;
		dstrect.h = srcrect.h;

		SDL_BlitSurface(animation, &srcrect, screen, &dstrect);

		if (animation_y != 0)
		{
			// blit remaining portion of animation higher up on screen than what we just blitted
			srcrect.x = 0;
			srcrect.y = animation->h - animation_y;
			srcrect.w = animation->w;
			srcrect.h = animation_y;

			dstrect.x = 0;
			dstrect.y = 0;
			dstrect.w = srcrect.w;
			dstrect.h = srcrect.h;

			SDL_BlitSurface(animation, &srcrect, screen, &dstrect);
		}

		animation_y += animation_vel;
		if (animation_y > animation_y_max)
		{
			animation_y = 0;
		}

		//blit menu border & BG
		rectangleRGBA(screen, ((screenw >> 1) - 80), 1, ((screenw >> 1) + 80), 58, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - 80 + 1;
		dstrect.y = 2;
		dstrect.w = ((screenw >> 1) + 80) - ((screenw >> 1) - 80) - 1;
		dstrect.h = 56;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));

		SFont_WriteCenter(screen, font_wh, 0, "Open2X LCD Calibration Utility");		

		// draw menu text
		SFont_Write(screen, font_wh, line_x, line_y[TIMING_LINE], "Timing:");		
		sprintf(tmp_str, "%d", current_timings.timing);
		SFont_Write(screen, font_wh, line_x + 45, line_y[TIMING_LINE], tmp_str);		
		SFont_Write(screen, font_wh, line_x + 73, line_y[TIMING_LINE], "(default is 1)");		
		SFont_Write(screen, font_wh, line_x, line_y[CLOCKGEN_LINE], "ClockGen:");		
		if (current_timings.clockgen == CLOCKGEN_UPLL)
		{
			SFont_Write(screen, font_wh, line_x + 45, line_y[CLOCKGEN_LINE], "UPLL");		
		}
		else
		{
			SFont_Write(screen, font_wh, line_x + 45, line_y[CLOCKGEN_LINE], "FPLL");		
		}
		SFont_Write(screen, font_wh, line_x + 73, line_y[CLOCKGEN_LINE], "(default is UPLL)");		
		SFont_Write(screen, font_wh, line_x, line_y[QUIT_AND_SAVE_LINE], "Save and exit");		
		SFont_Write(screen, font_wh, line_x, line_y[QUIT_LINE], "Abandon changes and exit");		

		// draw cursor
		SFont_Write(screen, font_wh, line_x - 4, line_y[current_line], ">");
		SFont_Write(screen, font_wh, line_x - 6, line_y[current_line] - 1, "-");

		// update screen and wait a bit
		SDL_Flip(screen);
		SDL_Delay(menu_delay);

		if ((SDL_GetTicks() - last_input_time) > 150)
		{
			// 150ms has passed since last accepting input
			SDL_JoystickUpdate();

//			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_START)) 
//			{
//				quit = 1;
//				break;
//			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
			{
				switch (current_line)
				{
					case TIMING_LINE:
						current_timings.timing++;
						if (current_timings.timing > (current_timings.clockgen == CLOCKGEN_UPLL ?
								upll_max : fpll_max))
						{
							// loop around to minimum timing
							current_timings.timing = (current_timings.clockgen == CLOCKGEN_UPLL ?
									upll_min : fpll_min);
						}
						set_new_timings(current_timings);
						break;
					case CLOCKGEN_LINE:
						current_timings.clockgen = (current_timings.clockgen == CLOCKGEN_UPLL ?
								CLOCKGEN_FPLL : CLOCKGEN_UPLL);
						current_timings.timing = 1;	// since each has a different valid range
						set_new_timings(current_timings);
						break;
					case QUIT_AND_SAVE_LINE:
						quit = 1;
						wants_to_save = 1;
						break;
					case QUIT_LINE:
						quit = 1;
						break;
					default:
						break;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
			{
				switch (current_line)
				{
					case TIMING_LINE:
						current_timings.timing++;
						if (current_timings.timing > (current_timings.clockgen == CLOCKGEN_UPLL ?
								upll_max : fpll_max))
						{
							// loop around to minimum timing
							current_timings.timing = (current_timings.clockgen == CLOCKGEN_UPLL ?
									upll_min : fpll_min);
						}
						set_new_timings(current_timings);
						break;
					case CLOCKGEN_LINE:
						current_timings.clockgen = (current_timings.clockgen == CLOCKGEN_UPLL ?
								CLOCKGEN_FPLL : CLOCKGEN_UPLL);
						current_timings.timing = 1;	// since each has a different valid range
						set_new_timings(current_timings);
						break;
					default:
						break;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
			{
				switch (current_line)
				{
					case TIMING_LINE:
						current_timings.timing--;
						if (current_timings.timing < (current_timings.clockgen == CLOCKGEN_UPLL ?
								upll_min : fpll_min))
						{
							// loop around to maximum timing
							current_timings.timing = (current_timings.clockgen == CLOCKGEN_UPLL ?
									upll_max : fpll_max);
						}
						set_new_timings(current_timings);
						break;
					case CLOCKGEN_LINE:
						current_timings.clockgen = (current_timings.clockgen == CLOCKGEN_UPLL ?
								CLOCKGEN_FPLL : CLOCKGEN_UPLL);
						current_timings.timing = 1;	// since each has a different valid range
						set_new_timings(current_timings);
						break;
					default:
						break;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_UP))
			{
				current_line--;
				if (current_line < 0)
				{
					current_line = num_lines - 1;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_DOWN))
			{
				current_line++;
				if (current_line >= num_lines)
				{
					current_line = 0;
				}
			}
						
			last_input_time = SDL_GetTicks();
		}
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
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Cannot initialize SDL system: %s\n", SDL_GetError());
		return 0;
	}
	 
	joy = SDL_JoystickOpen(0);
	if (joy == NULL)
	{
		fprintf(stderr, "Cannot initialize GP2X joystick: %s\n", SDL_GetError());
	}

	if (!load_graphics()) {
		fprintf(stderr, "Error loading sprites/fonts.\n");
		return 0;
	}

	if (!init_gpio()) {
		fprintf(stderr, "Error mapping GPIO registers!\n");
		return 0;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	// Initialize new timings to default timings for now
	current_timings.clockgen = default_timings.clockgen;
	current_timings.timing = default_timings.clockgen;
	original_timings = current_timings;

	if (argc > 1) {
		// We have been passed command-line parameters
		switch (argv[1][1]) {
			case 'h':
				// display help:
				printf("GUI LCD timings adjustment utility written by Senor Quack v1.0\n");
				printf("Copyright (C) 2008 Daniel Silsby\n\n");
				printf("  Portions of code adapted from:\n");
				printf("  CPU/LCD/RAM-Tuner for GP2X Version 2.0\n");
				printf("  Copyright (C) 2006 god_at_hell \n\n");
				printf("OPTIONS:\n");
				printf("-h\t\t:\tDisplay help and version info\n\n");
				printf("-f FILENAME\t:\tRestore saved settings from a file\n\n");
				printf("\t(if not specified, default is %s\n", settings_filename);
				return 0;
				break;
			case 'f':
				if (argc > 2)
				{
					// Good, we got passed a filename (presumably)
					if (strlen(argv[2]) < 254)
					{
						strcpy(settings_filename, argv[2]);
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

				break;
			default:
				// invalid parameter
				printf("Error: invalid parameter passed, see help by passing -h\n");
		} // switch
	} 

	// Initialize graphics, load sprits/fonts etc, mmap registers
	if (!initialize())
	{
		fprintf(stderr, "Aborting because of initialization errors.\n");
		return 1;
	}

	//  Return 0 on success, -1 on error.  Read settings into *timings structure.
	if (!read_saved_timings(settings_filename, &original_timings))
	{
		printf("Unable to read timings from %s, using defaults values.\n", settings_filename);
		original_timings.clockgen = default_timings.clockgen;
		original_timings.timing = default_timings.timing;
	}

	// current_timings will contain any possible new timings being tried out,
	// original_timings will contain the original settings as read from the file

	current_timings = original_timings;

	// RUN MENU LOOP
	if (menu())
	{
		// Returned 1, user wanted to save new values
		printf("User requested to save new LCD timings.\n");
		if (write_new_timings(settings_filename, current_timings))
		{
			printf("Successfully saved new values.\n");
		}
		else
		{
			printf("Problem occurred writing new values.\n");
		}

		sync();
	}
	else
	{
		// Returned 0, user wants to abandon new timings
		set_new_timings(original_timings);
		printf("Restored original timings.\n");
	}

	shutdown();

	return 0;
}

