/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created, Developing from April 2005
 */

#ifndef __DRAWCONTROL_H_
#define __DRAWCONTROL_H_

#include "typed.h"

void OnDrawMenu_BackGorund();
void OnDrawMenu_Icon(int cx, int cy, int icon, int status);
void ShowMenu();
void HideMenu();
void ShowLoding();
void HideLoding();


void OnDraw_MoveMenu(int posOld, int pos);
void OnDrawMenu_BackGorundButton(int posOld);

void OnHide_LargeNumber();
void OnHide_SmallNumber();
void OnDraw_SmallNumberText(int x, int y, int hour, int min);
void OnDraw_LargeNumberText(int x, int y, int hour, int min);
void OnDraw_LColon(int x, int y);
void OnDraw_SColon(int x, int y);
void OnDraw_LargeNumber(int x, int y, int num);
void OnDraw_SmallNumber(int x, int y, int num);
void OnDraw_VolumeIcon(int x, int y, int vol, bool flag);
void OnDraw_Volume(int vol);


#endif
