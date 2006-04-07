// hello.c - Dan Peori <peori@oopo.net>
// Copy all you want, please give me some credit.

 #include <stdio.h>

 #include <SDL.h>
 #include <SDL_ttf.h>
 #include <SDL_image.h>
 #include <SDL_mixer.h>

 SDL_Surface *video_surface = NULL;

 int draw_image(char *filename);

 int draw_text(char *filename, int size, char *text);

 //////////////////
 // MAIN PROGRAM //
 //////////////////

 int main(int argc, char **argv) {
  int result = 0;

  // Initialize the sdl library.
  result = SDL_Init(SDL_INIT_EVERYTHING);
  if (result < 0) { fprintf(stderr, "ERROR: Could not initialize the sdl library.\n"); return -1; }

  // Set the video mode.
  video_surface = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
  if (video_surface == NULL) { fprintf(stderr, "ERROR: Could not set the video mode.\n"); return -1; }

  // Draw the sample image.
  result = draw_image("media/image.jpg");
  if (result < 0) { fprintf(stderr, "ERROR: Could not draw the sample image.\n"); return -1; }

  // Draw the sample text.
  result = draw_text("media/font.ttf", 40, "Hello, World!");
  if (result < 0) { fprintf(stderr, "ERROR: Could not draw the sample text.\n"); return -1; }

  // Flip the video surface.
  result = SDL_Flip(video_surface);
  if (result < 0) { fprintf(stderr, "ERROR: Could not flip the video surface.\n"); return -1; }

  // Loop forever.
  for(;;) { usleep(1000); }

  // End program.
  return 0;

 }

 //////////////////////
 // SAMPLE FUNCTIONS //
 //////////////////////

 int draw_image(char *filename) {
  SDL_Surface *temp = NULL;

  // Load the image into a temporary surface.
  if ((temp = IMG_Load(filename)) == NULL) { return -1; }

  // Blit the temporary surface onto the video surface.
  if (SDL_BlitSurface(temp, NULL, video_surface, NULL) < 0) { return -1; }

  // Destroy the temporary surface.
  SDL_FreeSurface(temp);

  // End function.
  return 0;

 }

 int draw_text(char *filename, int size, char *text) {
  TTF_Font *font = NULL;
  SDL_Surface *temp = NULL;
  SDL_Color color = { 255, 255, 255, 255 };

  // Initialize the ttf library.
  if (TTF_Init() < 0) { return -1; }

  // Load the font.
  if ((font = TTF_OpenFont(filename, size)) == NULL) { return -1; }

  // Draw the text onto a temporary surface.
  if ((temp = TTF_RenderText_Blended(font, text, color)) == NULL) { return -1; }

  // Blit the temporary surface onto the video surface.
  if (SDL_BlitSurface(temp, NULL, video_surface, NULL) < 0) { return -1; }

  // Destroy the temporary surface.
  SDL_FreeSurface(temp);

  // Unload the font.
  TTF_CloseFont(font);

  // Shut down the ttg library.
  // if (TTF_Quit() < 0) { return -1; }

  // End function.
  return 0;

 }
