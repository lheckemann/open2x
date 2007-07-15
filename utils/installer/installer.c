#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	SDL_Surface *screen;
	screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_Surface *splash;
	splash = IMG_Load("splash.png");

	while(1){
		SDL_BlitSurface(splash, NULL, screen, NULL);
		SDL_Flip(screen);
		sleep(1);
	}
	return 0;
}
