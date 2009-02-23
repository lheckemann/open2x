//[*]----------------------------------------------------------------------------------------------------[*]
#ifndef __TYPED_H_
#define __TYPED_H_
//[*]----------------------------------------------------------------------------------------------------[*]
// 파일 탐색 모드
typedef enum 
{
	FILE_MODE			,
	FOLDER_MODE		    ,
	FOLDER_FILE_MODE	,
	FOLDER_MP3OGG_MODE  ,
	FOLDER_AVI_MODE	    ,
	FOLDER_IMG_MODE	    ,
	FOLDER_TXT_MODE	    ,
	FOLDER_GPE_MODE	    
}SeletMode;
//[*]----------------------------------------------------------------------------------------------------[*]
// 파일 포맷
typedef enum 
{
	FOLDER_FORMAT	,	// 폴더
	MP3_FORMAT		,	// 음악
	OGG_FORMAT		,
	AVI_FORMAT		,	// 동영상
	BMP_FORMAT		,	// 그림
	GIF_FORMAT		,
	PNG_FORMAT		,
	JPG_FORMAT		,
	TXT_FORMAT		,	// 텍스트 파일
	FILE_FORMAT		,	// 기타 파일
	GPE_FORMAT			// GP2X 실행파일
}FileFormat;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	PLAY_STATUS,
	PAUSE_STATUS,
	STOP_STATUS,
	EXIT_STAUTS
}VideoStatus;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	MOVIE_VIEW,
	FILE_VIEW,
	EXIT_VIEW
}ViewMode;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef struct 
{
	int 	nStartCount	;
	int 	nEndCount	;
	int 	nPosition	;
	int		nStatus		;
}SViewInfomation;
//[*]----------------------------------------------------------------------------------------------------[*]
// 파일 리스트
typedef struct
{
	char 		*szName;
	FileFormat 	nAttribute;
}SFileList;
//[*]----------------------------------------------------------------------------------------------------[*]
// 파일 정보
typedef struct
{
	char 		*szPath;
	SFileList 	*pList;
	int 		nCount;
}SDirInfomation;
//[*]----------------------------------------------------------------------------------------------------[*]
enum MAP_KEY
{
	VK_UP			,	// 0
	VK_UP_LEFT		,	// 1
	VK_LEFT			,	// 2
	VK_DOWN_LEFT	,	// 3
	VK_DOWN			,	// 4
	VK_DOWN_RIGHT	,	// 5
	VK_RIGHT		,	// 6
	VK_UP_RIGHT		,	// 7
	VK_START		,	// 8
	VK_SELECT		,	// 9
	VK_FL			,	// 10
	VK_FR			,	// 11	
	VK_FA			,	// 12	
	VK_FB			,	// 13	
	VK_FX			,	// 14	
	VK_FY			,	// 15	
	VK_VOL_UP		,	// 16	
	VK_VOL_DOWN		,	// 17	
	VK_TAT				// 18	
};	                	
//[*]----------------------------------------------------------------------------------------------------[*]
enum IDD_Menu
{
	PREV_FILE_BUTTON,
	NEXT_FILE_BUTTON,
	PREV_SEEK_BUTTON,
	NEXT_SEEK_BUTTON,
	PLAY_BUTTON,
	PAUSE_BUTTON,
	STOP_BUTTON,
	OPEN_BUTTON,
	EQ_BUTTON
};
//[*]----------------------------------------------------------------------------------------------------[*]
typedef struct 
{
	int x;
	int y;
}Point;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	NORMAL_EQ,
	CLASSIC_EQ,
	JAZZ_EQ,
	POP_EQ
}Equlize;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	PREV_FILE_CMD,
	NEXT_FILE_CMD,
	PREV_SEEK_CMD,
	NEXT_SEEK_CMD,
	STOP_CMD,
	OPEN_CMD,
	EXIT_CMD
}NEXT_COMMAND;
//[*]----------------------------------------------------------------------------------------------------[*]
#define true	1
#define	false	0
typedef int bool;
//[*]----------------------------------------------------------------------------------------------------[*]
#endif
//[*]----------------------------------------------------------------------------------------------------[*]
