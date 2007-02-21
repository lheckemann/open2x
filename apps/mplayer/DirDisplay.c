/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created, Developing from April 2005
 */

#include "DirDisplay.h"
#include "config.h"

#include "SDL.h"
#include "SDL_image.h"

#define IMAGEPATH			"./imgcommon/"

#define IMG_FORLDER_PATH	IMAGEPATH"folder.png"
#define IMG_AVI_PATH		IMAGEPATH"avi.png"	
#define IMG_MP3_PATH		IMAGEPATH"mp3.png"	
#define IMG_OGG_PATH		IMAGEPATH"ogg.png"	
#define IMG_LINE_PATH		IMAGEPATH"selectbar.png"


void OnDraw_Dir(SDL_Surface *pScreen, SDL_Surface *pImgScreen, SViewInfomation infoView, SDirInfomation *infoDirList)
{
	if(pScreen == NULL || pImgScreen == NULL || infoDirList == NULL)
	{
		return;	
	}

	if(infoView.nStatus == 0)	OnDrawBase(pScreen, pImgScreen, infoView, infoDirList);
	else						OnDrawDirectory(pScreen, pImgScreen, infoView, infoDirList);
}


void OnDrawBase(SDL_Surface *pScreen, SDL_Surface *pImgScreen, SViewInfomation infoView, SDirInfomation *infoDirList)
{
	int 		i 			= 0;
	int 		j 			= 0;
	char 		path[256]	= {	0,	};
	SDL_Rect 	rect;

	SDL_Surface *pImg_SD	= NULL;
	SDL_Surface *pImg_NAND	= NULL;
	SDL_Surface *pImg_EXT	= NULL;
	
	if(pImgScreen)
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = pImgScreen->w;
		rect.h = pImgScreen->h;
		
		SDL_BlitSurface(pImgScreen, NULL, pScreen, NULL);		
	}
		
	if(infoView.nPosition == 0) {
		pImg_SD 	= IMG_Load(IMAGEPATH"sd_on.png");
		pImg_NAND 	= IMG_Load(IMAGEPATH"nand.png");
		pImg_EXT	= IMG_Load(IMAGEPATH"ext.png");
	} else if(infoView.nPosition == 1) {
		pImg_SD 	= IMG_Load(IMAGEPATH"sd.png");
		pImg_NAND 	= IMG_Load(IMAGEPATH"nand_on.png");
		pImg_EXT	= IMG_Load(IMAGEPATH"ext.png");
	} else {
		pImg_SD 	= IMG_Load(IMAGEPATH"sd.png");
		pImg_NAND 	= IMG_Load(IMAGEPATH"nand.png");
		pImg_EXT	= IMG_Load(IMAGEPATH"ext_on.png");
	}

	if(pImg_SD)
	{
		rect.x = 67;
		rect.y = 67;
		rect.w = pImg_SD->w;
		rect.h = pImg_SD->h;
	
		SDL_BlitSurface(pImg_SD, NULL, pScreen, &rect);
	}
	
	if(pImg_NAND)
	{
		rect.x = 67;
		rect.y = 100;
		rect.w = pImg_NAND->w;
		rect.h = pImg_NAND->h;
	
		SDL_BlitSurface(pImg_NAND, NULL, pScreen, &rect);
	}
		
	if(pImg_EXT)
	{
		rect.x = 67;
		rect.y = 123;
		rect.w = pImg_EXT->w;
		rect.h = pImg_EXT->h;
	
		SDL_BlitSurface(pImg_EXT, NULL, pScreen, &rect);
	}

    if(pImg_NAND)	SDL_FreeSurface(pImg_NAND);		
    if(pImg_SD	)	SDL_FreeSurface(pImg_SD	 );
    if(pImg_EXT )	SDL_FreeSurface(pImg_EXT );		
    pImg_NAND = NULL;
    pImg_SD	  = NULL;
	pImg_EXT  = NULL;
}


void OnDrawDirectory(SDL_Surface *pScreen, SDL_Surface *pImgScreen, SViewInfomation infoView, SDirInfomation *infoDirList)
{
	int 		i 			= 0;
	int 		j 			= 0;
	SDL_Rect 	rect;

	SDL_Surface *pImg_Folder	= NULL;
	SDL_Surface *pImg_Avi		= NULL;
	SDL_Surface *pImg_Mp3		= NULL;
	SDL_Surface *pImg_Ogg		= NULL;
	SDL_Surface *pImg_Line		= NULL;

	SDL_Surface *pImg_Icon		= NULL;
	
	pImg_Folder	= IMG_Load(IMG_FORLDER_PATH	);
	pImg_Avi	= IMG_Load(IMG_AVI_PATH		);
	pImg_Mp3	= IMG_Load(IMG_MP3_PATH		);
	pImg_Ogg	= IMG_Load(IMG_OGG_PATH		);
	pImg_Line	= IMG_Load(IMG_LINE_PATH	);
	
	SDL_BlitSurface(pImgScreen, NULL, pScreen, NULL);	
	
	
    for(i=infoView.nStartCount,j=0; i<infoView.nEndCount; i++,j++)
    {
    	
    	unsigned short *dest = NULL;		
		
		if(infoDirList->pList[i].nAttribute == FOLDER_FORMAT)
			dest = OnUTF8ToUnicode(infoDirList->pList[i].szName, 0);
		else	
			dest = OnUTF8ToUnicode(infoDirList->pList[i].szName, 4);

		
		
		switch(infoDirList->pList[i].nAttribute)
		{
			case FOLDER_FORMAT	:	pImg_Icon = pImg_Folder;	break;
			case AVI_FORMAT		:	pImg_Icon = pImg_Avi;		break;
			case MP3_FORMAT		:	pImg_Icon = pImg_Mp3;		break;
			case OGG_FORMAT		:	pImg_Icon = pImg_Ogg;		break;
		}
		
		if(pImg_Icon != NULL)
		{
			rect.x = 18;
			rect.y = 67+j*21;
			rect.w = pImg_Icon->w;
			rect.h = pImg_Icon->h;
	
			SDL_BlitSurface(pImg_Icon, NULL, pScreen, &rect);
		}
		


		DrawTextOut(pScreen, 38, 67+j*21, 320-38-10, dest, 0xFF, 0xFF, 0xFF);

		
		if(infoView.nPosition == i)	
		{               
			if(pImg_Line)
			{
				pImg_Icon = pImg_Line;
				rect.x = 40;
				rect.y = 81+j*21;
				rect.w = pImg_Icon->w;
				rect.h = pImg_Icon->h;
				
				SDL_BlitSurface(pImg_Icon, NULL, pScreen, &rect);
			}
		}
	
		free(dest);
    }

    if(pImg_Folder	)	SDL_FreeSurface(pImg_Folder	);
    if(pImg_Avi		)	SDL_FreeSurface(pImg_Avi	);
    if(pImg_Mp3		)	SDL_FreeSurface(pImg_Mp3	);
    if(pImg_Ogg		)	SDL_FreeSurface(pImg_Ogg	);
    if(pImg_Line	)	SDL_FreeSurface(pImg_Line	);
    
    pImg_Folder = NULL;
    pImg_Avi	= NULL;
    pImg_Mp3	= NULL;
    pImg_Ogg	= NULL;
    pImg_Line	= NULL;    
}
