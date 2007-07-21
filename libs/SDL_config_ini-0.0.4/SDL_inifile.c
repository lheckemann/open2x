#include "src/SDL_config.h"
#include "SDL_inifile.h"

//[*]----------------------------------------------------------------------------------------------------[*]
//[*] For INI Control(Simply SDL_config)                                                                 [*]
//[*]----------------------------------------------------------------------------------------------------[*]
static CFG_File INI_Cfg;
static int INI_Opened = 0;
static int INI_Changed = 0;
static char INI_File[4096] = {0, };
static char INI_Group[255] = {0, };
//[*]----------------------------------------------------------------------------------------------------[*]
int INI_Open(const char *file)
{
	if(INI_Opened) return 0;
	if(CFG_OK != CFG_OpenFile(file, &INI_Cfg))
		CFG_OpenFile(0, &INI_Cfg);
	strcpy(INI_File, file);
	INI_Group[0] = '\0';
	INI_Changed = 0;
	INI_Opened = 1;
}

void INI_Close(void)
{
	if(!INI_Opened) return;
	if(INI_Changed)
		CFG_SaveFile(INI_File, CFG_SORT_TYPE_CONTENT, CFG_COMPRESS_OUTPUT);
	CFG_CloseFile(&INI_Cfg);
	INI_Opened = 0;
	INI_Changed = 0;
	INI_File[0] = '\0';
	INI_Group[0] = '\0';
}
//[*]----------------------------------------------------------------------------------------------------[*]
int INI_ReadInt(const char *group, const char *entry, const int defval)
{
	if(!INI_Opened) return defval;
	if(strcmp(INI_Group, group) != 0) {
		if(CFG_OK != CFG_SelectGroup(group, CFG_False))
			return defval;
		strcpy(INI_Group, group);
	}
	return CFG_ReadInt(entry, defval);
}

int INI_ReadBool(const char *group, const char *entry, const int defval)
{
	if(!INI_Opened) return defval;
	if(strcmp(INI_Group, group) != 0) {
		if(CFG_OK != CFG_SelectGroup(group, CFG_False))
			return defval;
		strcpy(INI_Group, group);
	}
	return (CFG_ReadBool(entry, (defval ? CFG_True : CFG_False)) == CFG_True ? 1 : 0);
}

float INI_ReadFloat(const char *group, const char *entry, const float defval)
{
	if(!INI_Opened) return defval;
	if(strcmp(INI_Group, group) != 0) {
		if(CFG_OK != CFG_SelectGroup(group, CFG_False))
			return defval;
		strcpy(INI_Group, group);
	}
	return CFG_ReadFloat(entry, defval);
}

const char *INI_ReadText(const char *group, const char *entry, const char *defval)
{
	if(!INI_Opened) return defval;
	if(strcmp(INI_Group, group) != 0) {
		if(CFG_OK != CFG_SelectGroup(group, CFG_False))
			return defval;
		strcpy(INI_Group, group);
	}
	return CFG_ReadText(entry, defval);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void INI_WriteInt(const char *group, const char *entry, const int val)
{
	int saved;

	if(!INI_Opened) return;

	if(strcmp(INI_Group, group) != 0) {
		CFG_SelectGroup(group, CFG_True);
		strcpy(INI_Group, group);
	}

	if(CFG_OK == CFG_IntEntryExists(entry))
		saved = (CFG_ReadInt(entry, 0) != val);
	else
		saved = 1;

	if(saved) {
		CFG_WriteInt(entry, val);
		INI_Changed = 1;
	}
}

void INI_WriteBool(const char *group, const char *entry, const int val)
{
	int saved;

	if(!INI_Opened) return;

	if(strcmp(INI_Group, group) != 0) {
		CFG_SelectGroup(group, CFG_True);
		strcpy(INI_Group, group);
	}

	if(CFG_OK == CFG_IntEntryExists(entry))
		saved = (CFG_ReadBool(entry, CFG_True) != (val ? CFG_True : CFG_False));
	else
		saved = 1;

	if(saved) {
		CFG_WriteBool(entry, (val ? CFG_True : CFG_False));
		INI_Changed = 1;
	}
}

void INI_WriteFloat(const char *group, const char *entry, const float val)
{
	int saved;

	if(!INI_Opened) return;

	if(strcmp(INI_Group, group) != 0) {
		CFG_SelectGroup(group, CFG_True);
		strcpy(INI_Group, group);
	}

	if(CFG_OK == CFG_IntEntryExists(entry))
		saved = (CFG_ReadFloat(entry, 0) != val);
	else
		saved = 1;

	if(saved) {
		CFG_WriteFloat(entry, val);
		INI_Changed = 1;
	}
}

void INI_WriteText(const char *group, const char *entry, const char *val)
{
	int saved;

	if(!INI_Opened) return;

	if(strcmp(INI_Group, group) != 0) {
		CFG_SelectGroup(group, CFG_True);
		strcpy(INI_Group, group);
	}

	if(CFG_OK == CFG_IntEntryExists(entry))
		saved = (strcmp(CFG_ReadText(entry, ""), val) != 0);
	else
		saved = 1;

	if(saved) {
		CFG_WriteText(entry, val);
		INI_Changed = 1;
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
