/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created.
 *
 */

#ifndef FONTDISPLAY_H_
#define FONTDISPLAY_H_

#include "SDL.h"

int transcode(const char *from_code, const char *to_code, const char *src, char *target, int len);
int DrawFont(SDL_Surface *pScreen, int x, int y, unsigned short code, Uint8 r, Uint8 g, Uint8 b);
void SetPixel(SDL_Surface *pScreen, int x, int y, Uint8 r, Uint8 g, Uint8 b);
void DrawText(SDL_Surface *pScreen, int x, int y, unsigned short *code, int lencode, Uint8 r, Uint8 g, Uint8 b);
void DrawTextOut(SDL_Surface *pScreen, int x, int y, int scale, unsigned short *code, Uint8 r, Uint8 g, Uint8 b);
unsigned short* OnUTF8ToUnicode(const char *src, int last);

#endif
