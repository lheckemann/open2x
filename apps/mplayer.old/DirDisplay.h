/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created, Developing from April 2005
 */
#ifndef __DIRDISPLAY_H_
#define __DIRDISPLAY_H_

#include "SDL.h"
#include "typed.h"
#include "DirList.h"

void InitialViewInfomation();
void OnDraw_Dir(SDL_Surface *pScreen, SDL_Surface *pImgScreen, SViewInfomation infoView, SDirInfomation *infoDirList);
void OnDrawBase(SDL_Surface *pScreen, SDL_Surface *pImgScreen, SViewInfomation infoView, SDirInfomation *infoDirList);
void OnDrawDirectory(SDL_Surface *pScreen, SDL_Surface *pImgScreen, SViewInfomation infoView, SDirInfomation *infoDirList);

#endif


