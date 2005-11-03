//[*]------------------------------------------------------------------------------------------[*]
/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/
//[*]------------------------------------------------------------------------------------------[*]
#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif
//[*]------------------------------------------------------------------------------------------[*]
#include <stdlib.h>
#include <stdio.h>		/* For the definition of NULL */
#include <sys/types.h>	// For Device open
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>		// For Device read


#include "SDL_error.h"
#include "SDL_joystick.h"
#include "SDL_sysjoystick.h"
#include "SDL_joystick_c.h"
//[*]------------------------------------------------------------------------------------------[*]
#define MAX_JOYSTICKS	18		/* only 2 are supported in the multimedia API */
#define MAX_AXES		0		/* each joystick can have up to 2 axes */
#define MAX_BUTTONS		18		/* and 6 buttons                      */
#define	MAX_HATS		0

#define	JOYNAMELEN		7

#if 0
typedef enum {
	PEPC_VK_UP,				// 0	
	PEPC_VK_UP_LEFT,		// 1
	PEPC_VK_UP_RIGHT,		// 2
	PEPC_VK_DOWN,			// 3
	PEPC_VK_DOWN_LEFT,		// 4
	PEPC_VK_DOWN_RIGHT,		// 5
	PEPC_VK_LEFT,			// 6
	PEPC_VK_RIGHT,			// 7
	PEPC_VK_FA,				// 8
	PEPC_VK_FB,				// 9
	PEPC_VK_FX,				// 10
	PEPC_VK_FY,				// 11
	PEPC_VK_FL,				// 12
	PEPC_VK_FR,				// 13	
	PEPC_VK_SELECT,			// 14		
	PEPC_VK_START,			// 15
	PEPC_VK_VOL_UP,			// 16
	PEPC_VK_VOL_DOWN		// 17	
}MAP_KEY;	
#endif	

#if 1
#define	PEPC_VK_UP				(1<<0)	
#define	PEPC_VK_UP_LEFT			(1<<1)	
#define	PEPC_VK_UP_RIGHT		(1<<2)	
#define	PEPC_VK_DOWN			(1<<3)	
#define	PEPC_VK_DOWN_LEFT		(1<<4)	
#define	PEPC_VK_DOWN_RIGHT		(1<<5)	
#define	PEPC_VK_LEFT			(1<<6)	
#define	PEPC_VK_RIGHT			(1<<7)	
#define	PEPC_VK_FA				(1<<8)	
#define	PEPC_VK_FB				(1<<9)	
#define	PEPC_VK_FX				(1<<10)	
#define	PEPC_VK_FY				(1<<11)	
#define	PEPC_VK_FL				(1<<12)	
#define	PEPC_VK_FR				(1<<13)	
#define	PEPC_VK_SELECT			(1<<14)	
#define	PEPC_VK_START			(1<<15)	
#define	PEPC_VK_VOL_UP			(1<<16)	
#define	PEPC_VK_VOL_DOWN		(1<<17)	
#endif

static char *dev_name = "/dev/GPIO";

//[*]------------------------------------------------------------------------------------------[*]
struct joystick_hwdata
{
	int fd;
	int prev_buttons;
};
//[*]------------------------------------------------------------------------------------------[*]
/* Function to scan the system for joysticks.
 * This function should set SDL_numjoysticks to the number of available
 * joysticks.  Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
int SDL_SYS_JoystickInit(void)
{
	printf("SDL_SYS_JoystickInit\n");
	return 1;
}
//[*]------------------------------------------------------------------------------------------[*]
/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	return "PEP Joy";
}
//[*]------------------------------------------------------------------------------------------[*]
/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	int fd = 0;

	fd = open(dev_name, O_RDWR | O_NDELAY );
	if(fd < 0){      	
        printf( "GPIO OPEN FAIL\n"); 
        return -1;
	}
	
	joystick->hwdata = (struct joystick_hwdata *) malloc(sizeof(*joystick->hwdata));
	if (joystick->hwdata == NULL)
	{
		SDL_OutOfMemory();
		return -1;
	}
	joystick->hwdata->prev_buttons=0;

	// fill nbuttons, naxes, and nhats fields
	joystick->nbuttons = MAX_BUTTONS;
	joystick->naxes = MAX_AXES;
	joystick->nhats = MAX_HATS;
	
	joystick->hwdata->fd = fd;
	joystick->hwdata->prev_buttons = 0;

	return(0);
}
//[*]------------------------------------------------------------------------------------------[*]
/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
	int ret=0;
	unsigned long buff=0,prev_buttons=0, changed=0,buttons=0;
	
	ret = read(joystick->hwdata->fd, &buff, 4); 

	prev_buttons 	= joystick->hwdata->prev_buttons;
	changed 		= buff^prev_buttons; 
		
	if(changed & PEPC_VK_UP			) SDL_PrivateJoystickButton(joystick, 0, 
												(buff & PEPC_VK_UP			) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_UP_LEFT	) SDL_PrivateJoystickButton(joystick, 1, 
												(buff & PEPC_VK_UP_LEFT		) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_UP_RIGHT	) SDL_PrivateJoystickButton(joystick, 2, 
												(buff & PEPC_VK_UP_RIGHT	) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_DOWN		) SDL_PrivateJoystickButton(joystick, 3, 
												(buff & PEPC_VK_DOWN		) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_DOWN_LEFT	) SDL_PrivateJoystickButton(joystick, 4, 
												(buff & PEPC_VK_DOWN_LEFT	) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_DOWN_RIGHT	) SDL_PrivateJoystickButton(joystick, 5, 
												(buff & PEPC_VK_DOWN_RIGHT	) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_LEFT		) SDL_PrivateJoystickButton(joystick, 6, 
												(buff & PEPC_VK_LEFT		) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_RIGHT		) SDL_PrivateJoystickButton(joystick, 7, 
												(buff & PEPC_VK_RIGHT		) ? SDL_PRESSED : SDL_RELEASED);
	
	if(changed & PEPC_VK_FA			) SDL_PrivateJoystickButton(joystick, 8, 
												(buff & PEPC_VK_FA			) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_FB			) SDL_PrivateJoystickButton(joystick, 9, 
												(buff & PEPC_VK_FB			) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_FX			) SDL_PrivateJoystickButton(joystick, 10, 
												(buff & PEPC_VK_FX			) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_FY			) SDL_PrivateJoystickButton(joystick, 11, 
												(buff & PEPC_VK_FY			) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_FL			) SDL_PrivateJoystickButton(joystick, 12, 
												(buff & PEPC_VK_FL			) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_FR			) SDL_PrivateJoystickButton(joystick, 13, 
												(buff & PEPC_VK_FR			) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_START		) SDL_PrivateJoystickButton(joystick, 14, 
												(buff & PEPC_VK_START		) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_SELECT		) SDL_PrivateJoystickButton(joystick, 15, 
												(buff & PEPC_VK_SELECT		) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_VOL_UP		) SDL_PrivateJoystickButton(joystick, 16, 
												(buff & PEPC_VK_VOL_UP		) ? SDL_PRESSED : SDL_RELEASED);
	if(changed & PEPC_VK_VOL_DOWN	) SDL_PrivateJoystickButton(joystick, 17, 
												(buff & PEPC_VK_DOWN		) ? SDL_PRESSED : SDL_RELEASED);
	

#if 0
	if(changed == PEPC_VK_UP			) SDL_PrivateJoystickButton(joystick, 0,  SDL_PRESSED);
	if(changed == PEPC_VK_UP_LEFT		) SDL_PrivateJoystickButton(joystick, 1,  SDL_PRESSED);
	if(changed == PEPC_VK_UP_RIGHT		) SDL_PrivateJoystickButton(joystick, 2,  SDL_PRESSED);
	if(changed == PEPC_VK_DOWN			) SDL_PrivateJoystickButton(joystick, 3,  SDL_PRESSED);
	if(changed == PEPC_VK_DOWN_LEFT		) SDL_PrivateJoystickButton(joystick, 4,  SDL_PRESSED);
	if(changed == PEPC_VK_DOWN_RIGHT	) SDL_PrivateJoystickButton(joystick, 5,  SDL_PRESSED);
	if(changed == PEPC_VK_LEFT			) SDL_PrivateJoystickButton(joystick, 6,  SDL_PRESSED);
	if(changed == PEPC_VK_RIGHT			) SDL_PrivateJoystickButton(joystick, 7,  SDL_PRESSED);
												
	if(changed == PEPC_VK_FA			) SDL_PrivateJoystickButton(joystick, 8,  SDL_PRESSED);
	if(changed == PEPC_VK_FB			) SDL_PrivateJoystickButton(joystick, 9,  SDL_PRESSED);
	if(changed == PEPC_VK_FX			) SDL_PrivateJoystickButton(joystick, 10, SDL_PRESSED);
	if(changed == PEPC_VK_FY			) SDL_PrivateJoystickButton(joystick, 11, SDL_PRESSED);
	if(changed == PEPC_VK_FL			) SDL_PrivateJoystickButton(joystick, 12, SDL_PRESSED);
	if(changed == PEPC_VK_FR			) SDL_PrivateJoystickButton(joystick, 13, SDL_PRESSED);
	if(changed == PEPC_VK_START			) SDL_PrivateJoystickButton(joystick, 14, SDL_PRESSED);
	if(changed == PEPC_VK_SELECT		) SDL_PrivateJoystickButton(joystick, 15, SDL_PRESSED);
	if(changed == PEPC_VK_VOL_UP		) SDL_PrivateJoystickButton(joystick, 16, SDL_PRESSED);
	if(changed == PEPC_VK_VOL_DOWN		) SDL_PrivateJoystickButton(joystick, 17, SDL_PRESSED);
#endif	
	
	/*joystick->hwdata->prev_buttons = buttons;*/
	joystick->hwdata->prev_buttons = buff;
}
//[*]------------------------------------------------------------------------------------------[*]
/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	if (joystick->hwdata != NULL) 
	{
		close(joystick->hwdata->fd);
		// free system specific hardware data
		free(joystick->hwdata);
	}
}
//[*]------------------------------------------------------------------------------------------[*]
/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
	return;
}
//[*]------------------------------------------------------------------------------------------[*]
