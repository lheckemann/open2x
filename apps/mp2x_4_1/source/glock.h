/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. October, 2004
 */

#ifndef __GLOCK_H
#    define __GLOCK_H

#    include <SDL.h>
#    include <SDL_thread.h>
    

    typedef enum SEM_INIT_MODE
{ SEMPTY, SFULL } SEMMODE;      

typedef struct Sync_Lock_t 
{
	int            v;
	SDL_mutex * mutex;
	SDL_cond * cond;
} SyncLock;



int             bin_lock_init(SyncLock * s, SEMMODE mode);
int            count_lock_init(SyncLock * s, SEMMODE mode, int init_value);
int            lock_take(SyncLock * s);
int            lock_give(SyncLock * s);
int            lock_value(SyncLock * s);
void           set_lock_value (SyncLock * s, SEMMODE mode);
int            lock_destroy(SyncLock * s);


#endif
