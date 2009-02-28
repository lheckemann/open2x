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
//#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <errno.h>
#include	<SDL.h>
#include	<SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include	"SFont.h"	

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

#define MINPRESSURE (35000)	// Any pressures below this are probably inaccurate
#define OFFSET 30					// Offset from edges of screen the readings are taken

typedef struct {
  unsigned short pressure;
  unsigned short x;
  unsigned short y;
  unsigned short pad;
} ts_sample;

extern int errno;

char *settings_filename = "/etc/pointercal";
int ts_dev;
ts_sample samples[2];			// We read two samples to calibrate


//global surfaces
SDL_Surface* screen;				// framebuffer
SDL_Surface* background;		
SDL_Surface* target;				// image to show where to tap the stylus

//global fonts
SFont_Font *font_wh; //white

//global joysticks
SDL_Joystick* joy;


//global constants
const int screenw = 320;
const int screenh = 240;
const int screenbpp = 16;
const int screenflags = SDL_DOUBLEBUF;
 

//  Return 1 on success, 0 on error.  Write settings from timings structure.
int write_calvals(const char *filename, ts_sample *samples_read)
{
	FILE *handle;
	char buf[100];
	int a, b, c, d, e, f, s, left, right, top, bottom;

	s = 1 << 16;

	left = samples_read->x;
	top = samples_read->y;

	samples_read++;			// advance to second sample in array

	right = samples_read->x;
	bottom = samples_read->y;

	a = s * (OFFSET - (319 - OFFSET)) / (left - right);
	b = 0;
	c = s * OFFSET - a * left;

	d = 0;
	e = s * (OFFSET - (239 - OFFSET)) / (top - bottom);
	f = s * OFFSET - e * top;

	handle = fopen(filename, "w");
	if (handle == NULL)
	{
	  printf("Error opening file for writing: %s\n", filename);
	  return 0;
	}
	else
	{
	  printf("Writing new calibration values to file: %s\n", filename);
	}

	sprintf(buf, "%d %d %d %d %d %d %d", a, b, c, d, e, f, s); 
	fprintf(handle, buf);

	return (fclose(handle) == 0);
}


int load_graphics(void)
{
	SDL_Surface *tmp_sur, *tmp_sur2;

	// Background image 
	tmp_sur = IMG_Load("img/bg.png");
	if (!tmp_sur)
	{
		fprintf(stderr, "Fatal error loading: img/bg.png\n");
		return 0;
	}
	background = SDL_DisplayFormat(tmp_sur);
	SDL_FreeSurface(tmp_sur);
	
	// Target image 
	tmp_sur = IMG_Load("img/target.png");
	if (!tmp_sur)
	{
		fprintf(stderr, "Fatal error loading: img/target.png\n");
		return 0;
	}
	target = SDL_DisplayFormat(tmp_sur);
	SDL_FreeSurface(tmp_sur);
	SDL_SetColorKey(target, SDL_SRCCOLORKEY, SDL_MapRGB(screen->format, 255, 0, 255));

	
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
	SDL_FreeSurface(target);

	if (font_wh) 
	{
		SFont_FreeFont(font_wh);
	}
}

// Read a touchscreen sample. Return 1 if sample was read successfully, 0 if not.
int read_ts(int handle, ts_sample *sample)
{
	return(read(handle, sample, sizeof(ts_sample)) == sizeof(ts_sample));
}

// Returns 1 if user completed calibration or 0 if user requested to abort the calibration
int calibration_sequence(void)
{
	int quit = 0;
	int times_pressed = 0;		// how many times has the stylus been pressed?  if 2, we're done

	SDL_Rect	dstrect;

	const int line_y[4] = { 13, 23, 33, 43 };		// Four lines on which we can place text

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
	}

	while (!quit)
	{
		// calibration loop

		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);

		//blit menu border & BG
		rectangleRGBA(screen, ((screenw >> 1) - 80), 1, ((screenw >> 1) + 80), 58, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - 80 + 1;
		dstrect.y = 2;
		dstrect.w = ((screenw >> 1) + 80) - ((screenw >> 1) - 80) - 1;
		dstrect.h = 56;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));

		SFont_WriteCenter(screen, font_wh, 0, "Open2X Touchscreen Calibration");		

		// draw menu text
		SFont_WriteCenter(screen, font_wh, line_y[3], "Press any button to abort");		

		if (times_pressed == 0)
		{
			SFont_WriteCenter(screen, font_wh, line_y[1], "Tap stylus on target");		
			// draw first target
			dstrect.x = OFFSET - (target->w >> 1);
			dstrect.y = OFFSET - (target->h >> 1);
			dstrect.w = target->w;
			dstrect.h = target->h;
			SDL_BlitSurface(target, NULL, screen, &dstrect);
		}
		else if (times_pressed == 1)
		{
			SFont_WriteCenter(screen, font_wh, line_y[1], "Tap stylus on next target");		
			// draw second target
			dstrect.x = (319 - OFFSET) - (target->w >> 1);
			dstrect.y = (239 - OFFSET) - (target->h >> 1);
			dstrect.w = target->w;
			dstrect.h = target->h;
			SDL_BlitSurface(target, NULL, screen, &dstrect);
		}

		ts_sample sample;
		if (read_ts(ts_dev, &sample))
		{
			// successfully read a sample
			if (sample.pressure > MINPRESSURE)
			{
				samples[times_pressed] = sample;
				times_pressed++;
				// don't move on until stylus is released for a period of time
				int stylus_still_pressed = 1;
				while (stylus_still_pressed)
				{
					if (read_ts(ts_dev, &sample) && sample.pressure == 0)
					{
						stylus_still_pressed = 0;
					}
					SDL_Delay(20);
				}

				if (times_pressed == 1)
				{
					// animate the target moving from one spot to another
					const int num_steps = 100;
					float x_incr = (float)(((319 - OFFSET) - (target->w >> 1)) - OFFSET) / 
											(float)num_steps;
					float y_incr = (float)(((239 - OFFSET) - (target->h >> 1)) - OFFSET) / 
											(float)num_steps;
					float dst_x = OFFSET - (target->w >> 1);
					float dst_y = OFFSET - (target->h >> 1);
					dstrect.w = target->w;
					dstrect.h = target->h;

					for (i = 0; i < num_steps; i++)
					{
						dstrect.x = dst_x;
						dstrect.y = dst_y;
						SDL_BlitSurface(background, NULL, screen, NULL);
						SDL_BlitSurface(target, NULL, screen, &dstrect);
						SDL_Flip(screen);
						dst_x += x_incr;
						dst_y += y_incr;
					}
				}
//				SDL_Delay(500);
			}
		}

		if (times_pressed == 2)
		{
			quit = 1;
		}	

		// update screen and wait a bit
		SDL_Flip(screen);
		SDL_Delay(10);

		SDL_JoystickUpdate();
		for (i = 0; i < NUMBUTTONS; i++)
		{
			if (SDL_JoystickGetButton(joy, i))
			{
				quit = 1;
			}
		}
	}

	return (times_pressed == 2);
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

	ts_dev = open("/dev/touchscreen/wm97xx",O_RDONLY|O_NDELAY);
	if (ts_dev == -1)
	{
		fprintf(stderr, "Error opening touchscreen device.\n");
		return 0;
	}

	return 1;
}

void shutdown(void)
{
	close(ts_dev);
	unload_graphics();
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	// Initialize graphics, load sprits/fonts etc, open touchscreen device
	if (!initialize())
	{
		fprintf(stderr, "Aborting because of initialization errors.\n");
		return 1;
	}

	// RUN CALIBRATION
	if (calibration_sequence())
	{
		// Returned 1, user completed calibration
		if (write_calvals(settings_filename, samples))
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
		// Returned 0, user aborted calibration sequence
		printf("User aborted.\n");
	}

	shutdown();

	return 0;
}

