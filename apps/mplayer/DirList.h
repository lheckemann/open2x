/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created, Developing from April 2005
 */

#ifndef __DIRLIST_H_
#define __DIRLIST_H_

#include <dirent.h>
#include "typed.h"

int GetDirInfo(SDirInfomation *infoDir, char *dir, SeletMode mode);
void DeleteDirInfo(SDirInfomation *infoDir);

#endif