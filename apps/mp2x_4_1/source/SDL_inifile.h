//senquack - stuck this here so we can compile correctly (this is a GPH-specific
//	library and really has little to do with SDL and isn't in the standard open2x
//	toolchain)
//
#ifndef __SDL_INIFILE_H__
#define __SDL_INIFILE_H__

#ifdef __cplusplus
extern "C" {
#endif

int INI_Open(const char *file);
void INI_Close(void);

int INI_ReadInt(const char *group, const char *entry, const int defval);
int INI_ReadBool(const char *group, const char *entry, const int defval);
float INI_ReadFloat(const char *group, const char *entry, const float defval);
const char *INI_ReadText(const char *group, const char *entry, const char *defval);

void INI_WriteInt(const char *group, const char *entry, const int val);
void INI_WriteBool(const char *group, const char *entry, const int val);
void INI_WriteFloat(const char *group, const char *entry, const float val);
void INI_WriteText(const char *group, const char *entry, const char *val);

#ifdef __cplusplus
} /* ends extern "C" */
#endif

#endif
