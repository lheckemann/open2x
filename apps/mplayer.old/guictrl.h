/*
 *  - DIGNSYS Inc. < www.dignsys.com >
 *
 *    => Created.
 */

#ifndef __GUICTRL_H_
#define __GUICTRL_H_

#include <SDL.h>
#include <SDL_image.h>

#include "typed.h"
#include "filelistview.h"

void OnDraw_SteroMono(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int flag);
void OnDraw_Repeat(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int flag);
void OnDraw_Shuf(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int flag);
void OnDraw_Equlize(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, Equlize eq);
void OnDraw_Infomation(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int x, int y, int nKbps, int nKhz);
void OnDraw_MoveProgress(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int Percent);
void OnUpdateBitmap(char *path, SDL_Surface *pBackScreen, Point point);
void OnDrawBitmap(char *path, SDL_Surface *pBackScreen, Point point);
void OnDraw_PlayTime(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int x, int y, int nMinute_Current, int nSecond_Current, int nMinute_End, int nSecond_End);
void OnDraw_Title(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int x, int y, int pos, unsigned char *name);
void DrawToolBarIcon(SDL_Surface *pScreen, char *path, int x, int y);
void HideToolBar();
void ShowToolBar();

#endif

