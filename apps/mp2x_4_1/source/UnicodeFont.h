#ifndef __UNICODEFONT_H_
#define __UNICODEFONT_H_

#ifdef WIN32
#define DECLSPEC __declspec(dllexport)
#endif


#ifdef __cplusplus
extern "C" {
#endif

//senquack
#ifdef WIN32
extern DECLSPEC int GetFont(unsigned short code, unsigned short *data, int *heigth, int *width);
#else
extern int GetFont(unsigned short code, unsigned short *data, int *heigth, int *width);
#endif


#ifdef __cplusplus
}
#endif

#endif
