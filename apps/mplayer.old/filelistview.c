/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created.
 *
 */

#include "filelistview.h"
#include "DirList.h"
#include "DirDisplay.h"
#include "typed.h"
#include "gvlib_export.h"	

#define IMAGEPATH			"./imgmovie/"
#define FILELIST_COUNT 		8
#define CLOCKADJUST_COUNT	3
#define IMG_BODY_MAIN 		IMAGEPATH"body.png"
#define IMG_FULL_ICON 		IMAGEPATH"full.png"
#define IMG_NORM_ICON 		IMAGEPATH"normal.png"
#define IMG_SAVE_ICON 		IMAGEPATH"save.png"
#define SHOWFILETYPE		FOLDER_AVI_MODE		

#define FCLK_200		10
#define FCLK_166		11
#define FCLK_133		12
#define FCLK_100		13
#define FCLK_78			14
#define FCLK_64			15
#define FCLK_DEFAULT	16

SViewInfomation 		infoView;					
SDirInfomation 			infoDir;					
static SDL_Surface 		*pImgBackGround = NULL;		
static SDL_Surface		*pImgClockIcon[3] = {NULL, };
extern ViewMode			viewmode;
extern int				bFileSelect;
extern char    			*ifile;
extern VideoStatus 		vstate;
extern int 				quit_event;	
extern int 				pause_flag;
extern NEXT_COMMAND		g_Command;			
extern SDL_Joystick 	*m_pJoy;
extern int				cpuclock;
const int AdjustClock[CLOCKADJUST_COUNT] = {FCLK_200, FCLK_166, FCLK_100};

void FileView_Initialize()
{
	pImgBackGround = IMG_Load(IMG_BODY_MAIN);								
	if(pImgBackGround == NULL)
		printf("no load %s\n", IMG_BODY_MAIN);
	pImgClockIcon[0] = IMG_Load(IMG_FULL_ICON);
	pImgClockIcon[1] = IMG_Load(IMG_NORM_ICON);
	pImgClockIcon[2] = IMG_Load(IMG_SAVE_ICON);
}


void FileView_Exitialize()
{
	int i;
	SDL_FreeSurface(pImgBackGround);
	pImgBackGround = NULL;

	for(i=0;i<CLOCKADJUST_COUNT;i++) {
		SDL_FreeSurface(pImgClockIcon[i]);
		pImgClockIcon[i] = NULL;
	}
}


void FileView_OnDraw(SDL_Surface *pScreen)
{
	OnDraw_Dir(pScreen, pImgBackGround, infoView, &infoDir);
	if(cpuclock < 0)cpuclock = CLOCKADJUST_COUNT - 1;
	if(cpuclock >= CLOCKADJUST_COUNT)cpuclock = 0;
	SDL_BlitSurface(pImgClockIcon[cpuclock], NULL, pScreen, NULL);
	SDL_UpdateRect(pScreen, 0, 0, 0, 0);
}


void FileView_OnTimerProc()
{
}


void FileView_OnProc(SDL_Surface *pScreen, SDL_Event *pEvent, SDL_Event *pOpenEvent)
{
	switch(pEvent->type) 
	{
		case SDL_JOYBUTTONDOWN :
			FileView_OnJoystickDown(pScreen, pEvent->jbutton.button, pOpenEvent);
			break;
		case SDL_JOYBUTTONUP :
			FileView_OnJoystickUp(pScreen, pEvent->jbutton.button, pOpenEvent);
			break;
		case SDL_KEYDOWN :
			FileView_OnKeyDown(pScreen, pEvent->key.keysym.sym, pOpenEvent);
			break;
		case SDL_KEYUP :
			FileView_OnKeyUp(pScreen, pEvent->key.keysym.sym, pOpenEvent);
			break;
	    default:
			break;
	}
}


void FileView_OnKeyDown(SDL_Surface *pScreen, SDLKey key, SDL_Event *pOpenEvent)
{
	switch(key)
	{
		case SDLK_i   	:	FileView_OnCursorUp(pScreen);	break;
		case SDLK_k 	:	FileView_OnCursorDown(pScreen);	break;

		case SDLK_j 	:	FileView_OnMoveHighFolder(pScreen);		break;
		case SDLK_u		:	FileView_OnMoveSub_Execute(pScreen, pOpenEvent);	break;	

		case SDLK_y		:	
			FileView_Exitialize();
			viewmode = MOVIE_VIEW;
			break;
		case SDLK_n		:	
			break;
		case SDLK_b		:	
			FileView_Exitialize();
			viewmode = MOVIE_VIEW;
			break;
	}
}


void FileView_OnKeyUp(SDL_Surface *pScreen, SDLKey key, SDL_Event *pOpenEvent)
{
}


void FileView_OnJoystickDown(SDL_Surface *pScreen, Uint8 button, SDL_Event *pOpenEvent)
{
	switch(button)
	{
		case VK_UP   	:	
			FileView_OnCursorUp(pScreen);	
			break;
		case VK_DOWN 	:	
			FileView_OnCursorDown(pScreen);	
			break;

		case VK_FX		:	
		case VK_LEFT 	:	
			FileView_OnMoveHighFolder(pScreen);		
			break;
		case VK_TAT		:	
		case VK_FB		:	
			FileView_OnMoveSub_Execute(pScreen, pOpenEvent);	
			break;	
		case VK_FY		:
			cpuclock++;
			FileView_OnDraw(pScreen);
			break;
		case VK_START	:	
			FileView_Exitialize();
			viewmode = MOVIE_VIEW;
			break;
	}
}

void FileView_OnJoystickUp(SDL_Surface *pScreen, Uint8 button, SDL_Event *pOpenEvent)
{
}


void FileView_OnCursorUp(SDL_Surface *pScreen)
{
	

	
	if(infoDir.nCount == 0)	return;
	
	if(infoView.nPosition <= 0)		
	{
		infoView.nPosition = (infoView.nPosition-1+infoDir.nCount)%infoDir.nCount;
		infoView.nStartCount 	= infoDir.nCount-FILELIST_COUNT;
		infoView.nEndCount		= infoDir.nCount;
		
		if(infoView.nStartCount < 0)
			infoView.nStartCount = 0;
	}
	else
	{
		infoView.nPosition--;		
		
		if(infoView.nPosition < infoView.nStartCount)
		{
			infoView.nStartCount--;
			infoView.nEndCount--;
		}
	}

	
	FileView_OnDraw(pScreen);
}

void FileView_OnCursorDown(SDL_Surface *pScreen)
{
	
	
	if(infoDir.nCount == 0)	return;
	
	if(infoView.nPosition >= infoDir.nCount -1)
	{
		infoView.nPosition = 0;
		infoView.nStartCount = 0;
		infoView.nEndCount = infoDir.nCount;
		
		if(infoView.nEndCount > FILELIST_COUNT)
			infoView.nEndCount = FILELIST_COUNT;
	}	
	else
	{
		infoView.nPosition++;	
		
		if(infoView.nPosition >= infoView.nEndCount)
		{
			infoView.nStartCount++;
			infoView.nEndCount++;
		}
	}
	
	
	FileView_OnDraw(pScreen);
}

void FileView_OnMoveHighFolder(SDL_Surface *pScreen)
{
	int 	i 		= 0;
	int 	len 	= 0;
	char 	*szPath = NULL;		
	char 	*szName = NULL;		
	
	if(infoView.nStatus != 1)	return;
	
	
	len = strlen(infoDir.szPath);
	for(i=len; i>=0; i--)
		if(infoDir.szPath[i] == '/')
			break;

	
	szName = (char*)malloc(strlen((char*)(&(infoDir.szPath[i+1])))+1);
	strcpy(szName, ((char*)(&(infoDir.szPath[i+1]))));

	
	szPath = (char*)malloc(i+1);
	strncpy(szPath, infoDir.szPath, i);
	szPath[i] = '\0';
	
	
	if(strcasecmp("/mnt", szPath) != 0)		
	{
		DeleteDirInfo(&infoDir);						
	    GetDirInfo(&infoDir, szPath, SHOWFILETYPE);	

    	infoView.nStartCount 	= 0;
		infoView.nPosition 		= 0;
    	infoView.nStatus 		= 1;		
    	if(infoDir.nCount > FILELIST_COUNT)	infoView.nEndCount = FILELIST_COUNT;
    	else								infoView.nEndCount = infoDir.nCount;
    	
    	
    	for(i=0; i<infoDir.nCount; i++)
    	{
    		
			if(infoDir.pList[i].nAttribute == FOLDER_FORMAT)		
			{
    			if(strcasecmp(szName, infoDir.pList[i].szName) == 0)
    			{
					infoView.nPosition = i;
					break;
	    		}
	    	}
	    	else
	    	{
	    		break;
	    	}
    	}
    }
    else		
    {
		DeleteDirInfo(&infoDir);			
    	infoView.nStartCount 	= 0;
    	infoView.nStatus 		= 0;		
    	infoView.nEndCount 		= 3;
		infoDir.nCount			= 3;		
		
    	if(strcmp(szName, "sd") == 0)			infoView.nPosition = 0;
    	else if(strcmp(szName, "nand") == 0)	infoView.nPosition = 1;
		else									infoView.nPosition = 2;
    }
    
    if(szName != NULL)	free(szName);	
	if(szPath != NULL)	free(szPath);	
	
	
	FileView_OnDraw(pScreen);
}

void FileView_OnMoveSub_Execute(SDL_Surface *pScreen, SDL_Event *pOpenEvent)
{
	if(infoDir.nCount == 0)	return;			

	if(infoView.nStatus == 0)		
	{
		DeleteDirInfo(&infoDir);	
		if(infoView.nPosition == 0)      GetDirInfo(&infoDir, "/mnt/sd"	, SHOWFILETYPE);
		else if(infoView.nPosition == 1) GetDirInfo(&infoDir, "/mnt/nand", SHOWFILETYPE);
		else					         GetDirInfo(&infoDir, "/mnt/ext", SHOWFILETYPE);

    	infoView.nStartCount 	= 0;
		infoView.nPosition 		= 0;
    	infoView.nStatus 		= 1;		
    	if(infoDir.nCount > FILELIST_COUNT)	infoView.nEndCount = FILELIST_COUNT;
    	else								infoView.nEndCount = infoDir.nCount;
	}
	else			
	{
		
		if(infoDir.pList[infoView.nPosition].nAttribute == FOLDER_FORMAT)
		{
	    	char 	*szPath = NULL;
	    	int 	len 	= 0;
	    	int 	i 		= 0;
	    	
	    	szPath = (char*)malloc(strlen(infoDir.szPath)+ strlen(infoDir.pList[infoView.nPosition].szName) + 2);
	    	sprintf(szPath, "%s/%s", infoDir.szPath, infoDir.pList[infoView.nPosition].szName);

			DeleteDirInfo(&infoDir);
			GetDirInfo(&infoDir, szPath, SHOWFILETYPE);
			
			free(szPath);
			
	    	infoView.nStartCount 	= 0;
			infoView.nPosition 		= 0;
	    	infoView.nStatus 		= 1;		
	    	if(infoDir.nCount > FILELIST_COUNT)	infoView.nEndCount = FILELIST_COUNT;
	    	else								infoView.nEndCount = infoDir.nCount;
		}			
		else	
		{
			FileView_Exitialize();
			viewmode = MOVIE_VIEW;
			bFileSelect = 1;	

			if(cpuclock < 0)cpuclock = CLOCKADJUST_COUNT - 1;
			if(cpuclock >= CLOCKADJUST_COUNT)cpuclock = 0;
			SDL_SYS_JoystickGp2xSys(m_pJoy, AdjustClock[cpuclock]); // FCLK_200

			if(vstate != PLAY_STATUS)
			{
				
			    if(ifile != NULL)	free(ifile);	
			    ifile = (char*)malloc(strlen(infoDir.szPath) + strlen(infoDir.pList[infoView.nPosition].szName) + 2);		
				sprintf(ifile, "%s/%s", infoDir.szPath, infoDir.pList[infoView.nPosition].szName);							
			    pOpenEvent->type = EVENT_MOVIE_PLAY;
			    SDL_PushEvent(pOpenEvent);
			}
			else
			{
				pause_flag 	= 0;		
				quit_event 	= 1;		
				
			    if(ifile != NULL)	free(ifile);	
			    ifile = (char*)malloc(strlen(infoDir.szPath) + strlen(infoDir.pList[infoView.nPosition].szName) + 2);		
				sprintf(ifile, "%s/%s", infoDir.szPath, infoDir.pList[infoView.nPosition].szName);							
				g_Command = OPEN_CMD;
			}
			
			return;
		}
	}

	
	FileView_OnDraw(pScreen);
}

