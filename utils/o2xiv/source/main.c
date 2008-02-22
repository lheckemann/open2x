#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include "gp2x.h"

int main(int argc, char* argv[]) {
	SDL_Surface *screen = NULL;
	SDL_Surface *image = NULL;
	SDL_Surface *tmp = NULL;
	SDL_Surface *proc = NULL;
	SDL_Rect srcrect;
	SDL_Event event;
	int run = 1;
	int vx = 0;
	int vy = 0;
	double zoom = 1.0;
	double angle = 0;

	if(!argv[1]) {
		printf("Need to pass image as argument\n");
		return 1;
	}

	SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
	SDL_JoystickOpen(0);
	screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if(!screen) {
		printf("Couldn't init screen\n");
		return 2;
	}
	atexit(SDL_Quit);
	SDL_ShowCursor(SDL_DISABLE);

	/* TODO: Add throbber */

	image = IMG_Load(argv[1]);
	if(!image) {
		printf("Couldn't load image %s", argv[1]);
		return 3;
	}
	tmp = SDL_DisplayFormat(image);

	srcrect.x = srcrect.y = 0;
	srcrect.w = 320;
	srcrect.h = 240;

	while(run) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_JOYBUTTONDOWN) {
				switch(event.jbutton.button) {
					case GP2X_BUTTON_UP:
						vy-=10;
						break;

					case GP2X_BUTTON_DOWN:
						vy+=10;
						break;

					case GP2X_BUTTON_LEFT:
						vx-=10;
						break;

					case GP2X_BUTTON_RIGHT:
						vx+=10;
						break;

					/* TODO: Add throbber for zoom and rotate */
					case GP2X_BUTTON_VOLUP:
						if(zoom == -0.1) {
							zoom += 0.2;
						} else {
							zoom += 0.1;
						}
						srcrect.x = srcrect.y = 0;
						SDL_FreeSurface(tmp);
						proc = rotozoomSurface(image, angle, zoom, SMOOTHING_OFF);
						tmp = SDL_DisplayFormat(proc);
						SDL_FreeSurface(proc);
						break;

					case GP2X_BUTTON_VOLDOWN:
						if(zoom == 0.1) {
							zoom -= 0.2;
						} else {
							zoom -= 0.1;
						}
						srcrect.x = srcrect.y = 0;
						SDL_FreeSurface(tmp);
						proc = rotozoomSurface(image, angle, zoom, SMOOTHING_OFF);
						tmp = SDL_DisplayFormat(proc);
						SDL_FreeSurface(proc);
						break;

					case GP2X_BUTTON_A:
						angle -= 90;
						srcrect.x = srcrect.y = 0;
						SDL_FreeSurface(tmp);
						proc = rotozoomSurface(image, angle, zoom, SMOOTHING_OFF);
						tmp = SDL_DisplayFormat(proc);
						SDL_FreeSurface(proc);
						break;

					case GP2X_BUTTON_B:
						angle += 90;
						srcrect.x = srcrect.y = 0;
						SDL_FreeSurface(tmp);
						proc = rotozoomSurface(image, angle, zoom, SMOOTHING_OFF);
						tmp = SDL_DisplayFormat(proc);
						SDL_FreeSurface(proc);
						break;
				}
			}

			if(event.type == SDL_JOYBUTTONUP) {
				switch(event.jbutton.button) {
					case GP2X_BUTTON_UP:
					case GP2X_BUTTON_DOWN:
						vy = 0;
						break;

					case GP2X_BUTTON_LEFT:
					case GP2X_BUTTON_RIGHT:
						vx = 0;
						break;

					case GP2X_BUTTON_START:
						run = 0;
						break;
				}
			}
		}

		if(((srcrect.x + vx) < (tmp->w-320+10)) && ((srcrect.x+vx) > -10)) srcrect.x += vx;
		if(((srcrect.y + vy) < (tmp->h-240+10)) && ((srcrect.y+vy) > -10)) srcrect.y += vy;

		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
		SDL_BlitSurface(tmp, &srcrect, screen, NULL);
		SDL_Flip(screen);
	}

	SDL_FreeSurface(image);
	SDL_FreeSurface(tmp);

	return 0;
}

