/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created, Developing from April 2005
 */

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "DirList.h"


static int folder_select(const struct dirent *name)
{
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	
	if(S_ISDIR(statbuf.st_mode))	
	{
		if(strcmp(".", name->d_name) == 0 || strcmp("..", name->d_name) == 0)	return 0;
		else																	return 1;
	}
	
	return 0;	
}


static int file_select(const struct dirent *name)
{
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	
	if(S_ISDIR(statbuf.st_mode))	return 0;
	
	return 1;	
}


static int folder_file_select(const struct dirent *name)
{
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	if(S_ISDIR(statbuf.st_mode))
	{
		
		if(strcmp(".", name->d_name) == 0 || strcmp("..", name->d_name) == 0)	return 0;

		return 1;	
	}

	return 1;	
}


static int folder_mp3ogg_select(const struct dirent *name)
{
	char *ext = NULL;
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	if(S_ISDIR(statbuf.st_mode))
	{
		
		if(strcmp(".", name->d_name) == 0 || strcmp("..", name->d_name) == 0)	return 0;

		return 1;	
	}
	
	
	ext = (char*)&(name->d_name[strlen(name->d_name)-4]);
	if(strcasecmp(".mp3", ext) == 0 || strcasecmp(".ogg", ext) == 0)
		return 1;	

	
	return 0;	
}


static int folder_avi_select(const struct dirent *name)
{
	char 		*ext = NULL;
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	if(S_ISDIR(statbuf.st_mode))
	{
		
		if(strcmp(".", name->d_name) == 0 || strcmp("..", name->d_name) == 0)	return 0;

		return 1;	
	}
	
	
	ext = (char*)&(name->d_name[strlen(name->d_name)-4]);
	if(strcasecmp(".avi", ext) == 0)
		return 1;	

	
	return 0;	
}

static int folder_img_select(const struct dirent *name)
{
	char 		*ext = NULL;
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	if(S_ISDIR(statbuf.st_mode))
	{
		
		if(strcmp(".", name->d_name) == 0 || strcmp("..", name->d_name) == 0)	return 0;

		return 1;	
	}
	
	
	ext = (char*)&(name->d_name[strlen(name->d_name)-4]);
	if(
		strcasecmp(".bmp", ext) == 0 || 
		strcasecmp(".gif", ext) == 0 || 
		strcasecmp(".png", ext) == 0 || 
		strcasecmp(".jpg", ext) == 0  
	)
		return 1;	

	
	return 0;	
}

static int folder_txt_select(const struct dirent *name)
{
	char 		*data = NULL;
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	if(S_ISDIR(statbuf.st_mode))
	{
		
		if(strcmp(".", name->d_name) == 0 || strcmp("..", name->d_name) == 0)	return 0;

		return 1;	
	}
	
	
	data = (char*)&(name->d_name[strlen(name->d_name)-4]);
	if(strcasecmp(".txt", data) == 0)
		return 1;	

	
	return 0;	
}

static int folder_gpe_select(const struct dirent *name)
{
	char 		*data = NULL;
	struct stat statbuf;
	lstat(name->d_name, &statbuf);
	
	if(S_ISDIR(statbuf.st_mode))
	{
		
		if(strcmp(".", name->d_name) == 0 || strcmp("..", name->d_name) == 0)	return 0;

		return 1;	
	}
	
	data = (char*)&(name->d_name[strlen(name->d_name)-4]);
	if(strcasecmp(".gpe", data) == 0)
		return 1;	

	
	return 0;	
}


int GetDirInfo(SDirInfomation *infoDir, char *dir, SeletMode mode)
{
    int 			n 				= 0;
    int 			i				= 0;
    int 			pos				= 0;
	char 			*szFullPath		= NULL;
	struct dirent 	**namelist		= NULL;
	char 			*current_path 	= NULL;
	struct stat 	statbuf;

	
	
	
	current_path = (char*)get_current_dir_name();
	chdir(dir);
	
	
    infoDir->szPath = (char*)malloc(strlen(dir)+1);
    strcpy(infoDir->szPath, dir);	 
    
    
    switch(mode)
    {
		case FILE_MODE			:	n = scandir(infoDir->szPath, &namelist, file_select			, alphasort);	break;
		case FOLDER_MODE		:   n = scandir(infoDir->szPath, &namelist, folder_select		, alphasort);	break;
		case FOLDER_FILE_MODE	:   n = scandir(infoDir->szPath, &namelist, folder_file_select	, alphasort);	break;
		case FOLDER_MP3OGG_MODE :   n = scandir(infoDir->szPath, &namelist, folder_mp3ogg_select, alphasort);	break;
		case FOLDER_AVI_MODE	:   n = scandir(infoDir->szPath, &namelist, folder_avi_select	, alphasort);	break;
		case FOLDER_IMG_MODE	:   n = scandir(infoDir->szPath, &namelist, folder_img_select	, alphasort);	break;
		case FOLDER_TXT_MODE	:   n = scandir(infoDir->szPath, &namelist, folder_txt_select	, alphasort);	break;
		case FOLDER_GPE_MODE	:   n = scandir(infoDir->szPath, &namelist, folder_gpe_select	, alphasort);	break;
    	defualt : 
    		return;
    }
    infoDir->nCount = n;
	
	
    if(infoDir->nCount > 0)
    {
    	infoDir->pList = (SFileList*)malloc(sizeof(SFileList)*(n)); 
		
		
		
    	for(i=0,pos=0; i<infoDir->nCount; i++)
    	{
   			lstat(namelist[i]->d_name, &statbuf);
   			
   			if(S_ISDIR(statbuf.st_mode))
   			{
   				
	    		infoDir->pList[pos].szName = (char*)malloc(strlen(namelist[i]->d_name)+1);			
    			strcpy(infoDir->pList[pos].szName, namelist[i]->d_name);					
	    		infoDir->pList[pos].nAttribute = FOLDER_FORMAT; 	

	    		pos++;
   			}
    	}
		
    	for(i=0; i<infoDir->nCount; i++)
    	{
    		
   			lstat(namelist[i]->d_name, &statbuf);
   			
   			if(!S_ISDIR(statbuf.st_mode))
   			{
   				char *ext = NULL;
   				
	    		infoDir->pList[pos].szName = (char*)malloc(strlen(namelist[i]->d_name)+1);			
    			strcpy(infoDir->pList[pos].szName, namelist[i]->d_name);					
   				
   				ext = (char*)&(namelist[i]->d_name[strlen(namelist[i]->d_name)-4]);

				
				if(strcasecmp(".mp3", ext) == 0)		infoDir->pList[pos].nAttribute = MP3_FORMAT; 	
				else if(strcasecmp(".ogg", ext) == 0)	infoDir->pList[pos].nAttribute = OGG_FORMAT; 	
				
				else if(strcasecmp(".avi", ext) == 0)	infoDir->pList[pos].nAttribute = AVI_FORMAT; 	
				
				else if(strcasecmp(".bmp", ext) == 0)	infoDir->pList[pos].nAttribute = BMP_FORMAT; 	
				else if(strcasecmp(".gif", ext) == 0)	infoDir->pList[pos].nAttribute = GIF_FORMAT;	
				else if(strcasecmp(".png", ext) == 0)	infoDir->pList[pos].nAttribute = PNG_FORMAT; 	
				else if(strcasecmp(".jpg", ext) == 0)	infoDir->pList[pos].nAttribute = JPG_FORMAT; 	
				
				else if(strcasecmp(".txt", ext) == 0)	infoDir->pList[pos].nAttribute = TXT_FORMAT; 	
				
				else if(strcasecmp(".gpe", ext) == 0)	infoDir->pList[pos].nAttribute = GPE_FORMAT; 	
				
				else									infoDir->pList[pos].nAttribute = FILE_FORMAT; 	

	    		pos++;
   			}
    	}
    	
		for(i=0; i<infoDir->nCount; i++)
        	free(namelist[i]);
        free(namelist);
    }

	
	chdir(current_path);
	free(current_path);		

    return n;
}


void DeleteDirInfo(SDirInfomation *infoDir)
{
	int i = 0;
	
	
	for(i=0; i<infoDir->nCount && infoDir->pList != NULL; i++)
	{
		if(infoDir->pList[i].szName != NULL)
		{
			if(infoDir->pList[i].szName)	
				free(infoDir->pList[i].szName);
			infoDir->pList[i].szName = NULL;
		}
	}

	if(infoDir->pList	)	free(infoDir->pList	);
	if(infoDir->szPath	)	free(infoDir->szPath);
	
	infoDir->nCount = 0;
	infoDir->pList 	= NULL;
	infoDir->szPath = NULL;
}