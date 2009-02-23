/*              
 *  - DIGNSYS Inc. < www.dignsys.com >
 *             
 *    => Created.
 */

#include "guictrl.h"

#define IMAGEPATH "./imgmusic/"

extern SDL_Surface 	*g_pScreen;
extern SDL_Surface 	*g_pImg_Body;
extern int posCursor;
extern bool bMenuStatus;



void OnDraw_SteroMono(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int flag)
{
	SDL_Surface *img_Stero = NULL;
	SDL_Surface *img_Mono  = NULL;
	SDL_Rect rectStero = { 218, 38, 0, 0 };
	SDL_Rect rectMono  = { 266, 38, 0, 0 };

	
	
	img_Stero = IMG_Load(IMAGEPATH"streo_on.bmp");
	img_Mono  = IMG_Load(IMAGEPATH"mono_on.bmp" );
	
	if(img_Stero != NULL)
	{
		rectStero.w = img_Stero->w;
		rectStero.h = img_Stero->h;
		SDL_BlitSurface(pBodyScreen, &rectStero, pScreen, &rectStero);
		if(flag == 1) SDL_BlitSurface(img_Stero, NULL, pScreen, &rectStero);
		SDL_FreeSurface(img_Stero);
		SDL_UpdateRect(pScreen, rectStero.x, rectStero.y, rectStero.w, rectStero.h);

	}
	
	if(img_Mono != NULL)
	{
		rectMono.w = img_Mono->w;
		rectMono.h = img_Mono->h;
		SDL_BlitSurface(pBodyScreen, &rectMono , pScreen, &rectMono);
		if(flag == 0)	SDL_BlitSurface(img_Mono, NULL, pScreen, &rectMono);
		SDL_FreeSurface(img_Mono);
		SDL_UpdateRect(pScreen, rectMono.x , rectMono.y , rectMono.w , rectMono.h );
	}
}

void OnDraw_Repeat(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int flag)
{
	SDL_Surface *img_icon = NULL;
	SDL_Rect rect = { 218, 56, 0, 0 };
	
	
	img_icon = IMG_Load(IMAGEPATH"repeat_on.bmp");
	
	if(img_icon != NULL)
	{
		rect.w = img_icon->w;
		rect.h = img_icon->h;
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		if(flag == 1) SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
		SDL_UpdateRect(pScreen, rect.x, rect.y, rect.w, rect.h);
	}
}

void OnDraw_Shuf(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int flag)
{
	SDL_Surface *img_icon = NULL;
	SDL_Rect rect = { 266, 56, 0, 0 };
	
	
	img_icon = IMG_Load(IMAGEPATH"shuf_on.bmp");
	
	if(img_icon != NULL)
	{
		rect.w = img_icon->w;
		rect.h = img_icon->h;
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		if(flag == 1) SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
		SDL_UpdateRect(pScreen, rect.x, rect.y, rect.w, rect.h);
	}
}

void OnDraw_Equlize(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, Equlize eq)
{
	SDL_Surface *img_icon = NULL;
	SDL_Rect rect = { 218, 74, 0, 0 };
	
	switch(eq)
	{
		case NORMAL_EQ 	:	img_icon = IMG_Load(IMAGEPATH"normal_eq.bmp" 	);	break;
		case CLASSIC_EQ	:	img_icon = IMG_Load(IMAGEPATH"eq_classic.bmp"	);	break;
		case JAZZ_EQ	:	img_icon = IMG_Load(IMAGEPATH"eq_jazz.bmp"		);	break;
		case POP_EQ		:	img_icon = IMG_Load(IMAGEPATH"eq_pop.bmp"		);	break;
	}

	if(img_icon != NULL)
	{
		rect.w = img_icon->w;
		rect.h = img_icon->h;
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
		SDL_UpdateRect(pScreen, rect.x, rect.y, rect.w, rect.h);
	}
}

void OnDraw_Infomation(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int x, int y, int nKbps, int nKhz)
{
	SDL_Surface *img_icon	= NULL;
	SDL_Rect	rect;
	Point 		point; 
	char 		szKbps[10];
	char 		szKhz[10];
	char 		szPath[256];
	int 		i = 0;
	
	point.x = x;	point.y = y;
	
	sprintf(szKbps, "%d", nKbps	);
	sprintf(szKhz , "%d", nKhz	);
	
	
	sprintf(szPath, IMAGEPATH"%s", "khz.png");
	img_icon = IMG_Load(szPath);
	if(img_icon != NULL)
	{
		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
	
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}
	
	
	for(i=strlen(szKhz)-1; i>=0; i--)
	{
		sprintf(szPath, IMAGEPATH"s%01d.png", szKhz[i]-'0');
		img_icon = IMG_Load(szPath);
		if(img_icon == NULL)
			continue;

		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;

		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}
	
	
	sprintf(szPath, IMAGEPATH"%s", "sslash.png");
	img_icon = IMG_Load(szPath);
	if(img_icon != NULL)
	{
		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
	
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	sprintf(szPath, IMAGEPATH"%s", "kbps.png");
	img_icon = IMG_Load(szPath);
	if(img_icon != NULL)
	{
		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
			
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	for(i=strlen(szKbps)-1; i>=0; i--)
	{
		sprintf(szPath, IMAGEPATH"s%01d.png", szKbps[i]-'0');

		img_icon = IMG_Load(szPath);
		if(img_icon == NULL)
			continue;

		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;

		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}
	
	
	
	SDL_UpdateRect(pScreen, rect.x, rect.y, x - rect.x, rect.h);
}


void OnDraw_MoveProgress(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int Percent)
{
	SDL_Rect temprect	= {	0, 0, 100, 100	};
	SDL_Rect rect		= {	23, 164, 306 - 23, 170 - 164	};
	SDL_Rect rectDraw	= {	23, 164, 0, 170 - 164	};

	rectDraw.w = (int)((Percent / 100.0f) * rect.w);

	SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
	
	SDL_UpdateRect(pScreen, 20, 158, 307-20, 175-158);

	
	SDL_FillRect(pScreen, &rectDraw, SDL_MapRGB(pScreen->format, 250, 143, 9));
	SDL_UpdateRect(pScreen, rect.x, rect.y, rect.w, rect.h);
}


void OnUpdateBitmap(char *path, SDL_Surface *pBackScreen, Point point)
{
	SDL_Surface *img_icon	= NULL;
	SDL_Rect	rect;

	img_icon = IMG_Load(path);

	if(img_icon != NULL)
	{
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
	
		SDL_BlitSurface(img_icon, NULL, pBackScreen, &rect);
		SDL_FreeSurface(img_icon);
		SDL_UpdateRect(g_pScreen, rect.x, rect.y, rect.w, rect.h);
	}
}


void OnDrawBitmap(char *path, SDL_Surface *pBackScreen, Point point)
{
	SDL_Surface *img_icon	= NULL;
	SDL_Rect	rect;

	img_icon = IMG_Load(path);
	if(img_icon != NULL)
	{
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
	
		SDL_BlitSurface(img_icon, NULL, pBackScreen, &rect);
		SDL_FreeSurface(img_icon);
	}
}


void OnDraw_PlayTime(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int x, int y, int nMinute_Current, int nSecond_Current, int nMinute_End, int nSecond_End)
{
	SDL_Surface *img_icon	= NULL;
	SDL_Rect	rect;
	Point 		point;

	char 		szMinute_Current[10];
	char 		szSecond_Current[10];
	char 		szMinute_End[10];
	char 		szSecond_End[10];
	
	char 		szPath[256];
	int 		i = 0;
	
	point.x = x;
	point.y = y;
	
	sprintf(szMinute_Current, "%02d", nMinute_Current	);
	sprintf(szSecond_Current, "%02d", nSecond_Current	);
	sprintf(szMinute_End	, "%02d", nMinute_End		);
	sprintf(szSecond_End	, "%02d", nSecond_End		);

	
	for(i=strlen(szSecond_End)-1; i>=0; i--)
	{
		sprintf(szPath, IMAGEPATH"%01d.png", szSecond_End[i]-'0');

		img_icon = IMG_Load(szPath);
		if(img_icon == NULL)
		{
			continue;
		}

		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;

		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	sprintf(szPath, IMAGEPATH"%s", "colonb.png");
	img_icon = IMG_Load(szPath);
	if(img_icon != NULL)
	{
		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
	
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	for(i=strlen(szMinute_End)-1; i>=0; i--)
	{
		sprintf(szPath, IMAGEPATH"%01d.png", szMinute_End[i]-'0');

		img_icon = IMG_Load(szPath);
		if(img_icon == NULL)
		{
			continue;
		}

		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;

		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	sprintf(szPath, IMAGEPATH"%s", "slash.png");
	img_icon = IMG_Load(szPath);
	if(img_icon != NULL)
	{
		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
	
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	for(i=strlen(szSecond_Current)-1; i>=0; i--)
	{
		sprintf(szPath, IMAGEPATH"%01d.png", szSecond_Current[i]-'0');

		img_icon = IMG_Load(szPath);
		if(img_icon == NULL)
		{
			continue;
		}

		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;

		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	sprintf(szPath, IMAGEPATH"%s", "colonb.png");
	img_icon = IMG_Load(szPath);
	if(img_icon != NULL)
	{
		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;
	
		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	
	for(i=strlen(szMinute_Current)-1; i>=0; i--)
	{
		sprintf(szPath, IMAGEPATH"%01d.png", szMinute_Current[i]-'0');

		img_icon = IMG_Load(szPath);
		if(img_icon == NULL)
		{
			continue;
		}

		point.x -= img_icon->w;
		rect.x = point.x;
		rect.y = point.y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;

		SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}

	SDL_UpdateRect(pScreen, rect.x, rect.y, x - rect.x, rect.h);
	
}


void OnDraw_Title(SDL_Surface *pScreen, SDL_Surface *pBodyScreen, int x, int y, int pos, unsigned char *name)
{
	SDL_Surface *img_icon	= NULL;
	SDL_Rect	rect;
	Point 		point;
	unsigned short *dest = NULL;
	int 		i = 0;
	
	point.x = x;
	point.y = y;

	rect.x = point.x;
	rect.y = point.y;
	rect.w = 285;
	rect.h = 16;
	
	SDL_BlitSurface(pBodyScreen, &rect, pScreen, &rect);
	dest = OnUTF8ToUnicode(name, 0);
	DrawTextOut(pScreen, point.x, point.y, 290, dest, 0xFF, 0xFF, 0xFF);
	
	SDL_UpdateRect(pScreen, rect.x, rect.y, rect.w, rect.h);

	if(dest)	free(dest);
}

void DrawToolBarIcon(SDL_Surface *pScreen, char *path, int x, int y)
{
	SDL_Surface *img_icon	= NULL;
	SDL_Rect	rect;

	img_icon = IMG_Load(path);
	if(img_icon != NULL)
	{
		if(x != -1)	rect.x = x;
		else		rect.x = (320-img_icon->w)/2;
		rect.y = y;
		rect.w = img_icon->w;
		rect.h = img_icon->h;

		SDL_BlitSurface(img_icon, NULL, pScreen, &rect);
		SDL_FreeSurface(img_icon);
	}
}

void HideToolBar()
{
	SDL_Rect	rect;
	
	rect.x = 0;
	rect.y = 190;
	rect.w = 320;
	rect.h = 240 - 190;

	SDL_BlitSurface(g_pImg_Body, &rect, g_pScreen, &rect);
	SDL_UpdateRect(g_pScreen, rect.x, rect.y, rect.w, rect.h);
	
	bMenuStatus = false;
}

void ShowToolBar()
{
	
	SDL_Rect	rect;
	
	rect.x = 0;
	rect.y = 195;
	rect.w = 320;
	rect.h = 240 - 195;

	SDL_BlitSurface(g_pImg_Body, &rect, g_pScreen, &rect);
	
	
	DrawToolBarIcon(g_pScreen, "./imgmusic/bt.bmp"		,  22, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/nt.bmp"		,  50, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/bs.bmp"		,  76, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/ns.bmp"		, 103, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/play.bmp"	, 129, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/pause.bmp"	, 154, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/stop.bmp"	, 179, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/load.bmp"	, 205, 197);
	DrawToolBarIcon(g_pScreen, "./imgmusic/eq.bmp"		, 284, 201);
	
	switch(posCursor)
	{
		case 0 :	DrawToolBarIcon(g_pScreen, "./imgmusic/bt_on.bmp"	,  22, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/bt_text.bmp"		, -1, 223);	break;
		case 1 :	DrawToolBarIcon(g_pScreen, "./imgmusic/nt_on.bmp"	,  50, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/nt_text.bmp"		, -1, 223);	break;
		case 2 :	DrawToolBarIcon(g_pScreen, "./imgmusic/bs_on.bmp"	,  76, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/bs_text.bmp"		, -1, 223);	break;
		case 3 :	DrawToolBarIcon(g_pScreen, "./imgmusic/ns_on.bmp"	, 103, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/ns_text.bmp"		, -1, 223);	break;
		case 4 :	DrawToolBarIcon(g_pScreen, "./imgmusic/play_on.bmp"	, 129, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/play_text.bmp"		, -1, 223);	break;
		case 5 :	DrawToolBarIcon(g_pScreen, "./imgmusic/pause_on.bmp", 154, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/pause_text.bmp"		, -1, 223);	break;
		case 6 :	DrawToolBarIcon(g_pScreen, "./imgmusic/stop_on.bmp"	, 179, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/stop_text.bmp"		, -1, 223);	break;
		case 7 :	DrawToolBarIcon(g_pScreen, "./imgmusic/load_on.bmp"	, 205, 197);	DrawToolBarIcon(g_pScreen, "./imgmusic/load_text.bmp"		, -1, 223);	break;
		case 8 :	DrawToolBarIcon(g_pScreen, "./imgmusic/eq_on.bmp"	, 284, 201);	DrawToolBarIcon(g_pScreen, "./imgmusic/eq_normal_text.bmp"		, -1, 223);	break;
	};              

	SDL_UpdateRect(g_pScreen, rect.x, rect.y, rect.w, rect.h);
	
	bMenuStatus = true;
}

