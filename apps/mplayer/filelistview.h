/*
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created.
 *
 */

#ifndef __FILELISTVIEW_H_
#define __FILELISTVIEW_H_

#include <SDL.h>
#include <SDL_image.h>

void FileView_Initialize();
void FileView_Exitialize();
void FileView_OnDraw(SDL_Surface *pScreen);
void FileView_OnTimerProc();
void FileView_OnProc(SDL_Surface *pScreen, SDL_Event *pEvent, SDL_Event *pOpenEvent);
void FileView_OnKeyDown(SDL_Surface *pScreen, SDLKey key, SDL_Event *pOpenEvent);
void FileView_OnKeyUp(SDL_Surface *pScreen, SDLKey key, SDL_Event *pOpenEvent);
void FileView_OnJoystickDown(SDL_Surface *pScreen, Uint8 button, SDL_Event *pOpenEvent);
void FileView_OnJoystickUp(SDL_Surface *pScreen, Uint8 button, SDL_Event *pOpenEvent);
void FileView_OnCursorUp(SDL_Surface *pScreen);
void FileView_OnCursorDown(SDL_Surface *pScreen);
void FileView_OnMoveHighFolder(SDL_Surface *pScreen);
void FileView_OnMoveSub_Execute(SDL_Surface *pScreen, SDL_Event *pOpenEvent);

#endif
