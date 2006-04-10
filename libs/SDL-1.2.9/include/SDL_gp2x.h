/*
 * Non-standard SDL header.
 *
 * This header gives access to a few extra features of the GP2X hardware
 * that I added to the video driver.
 */

#ifndef SDL_GP2X__H
#define SDL_GP2X__H

#ifdef __cplusplus
extern "C" {
#endif
  
void SDL_GP2X_SetMonoColours(int background, int foreground);
int  SDL_GP2X_GetPhysicalScreenSize(SDL_Rect *size);
void SDL_GP2X_Display(SDL_Rect *area);
void SDL_GP2X_InitializeCursor();
void SDL_GP2X_SetCursorColour(WMcursor *wcursor,
			      int bred, int bgreen, int bblue, int balpha,
			      int fred, int fgreen, int fblue, int falpha);
void SDL_GP2X_DefineRegion(int region, SDL_Rect *area);
void SDL_GP2X_ActivateRegion(int region, int activate);
void SDL_GP2X_MiniDisplay(int x, int y);

#ifdef __cplusplus
}
#endif

#endif
