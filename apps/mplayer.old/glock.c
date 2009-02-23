/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. October, 2004
 */

#include "glock.h"

int lock_take (SyncLock * s)
{
    int         value_after_op;

   	SDL_LockMutex(s->mutex);
   	while( (s->v) <= 0 )  
  		SDL_CondWait(s->cond, s->mutex);

	
    (s->v)--;
    value_after_op = s->v;

	SDL_UnlockMutex(s->mutex);

    return (value_after_op);
}

int lock_give (SyncLock * s)
{
    int         value_after_op;

   	SDL_LockMutex(s->mutex);
    (s->v)++;
    value_after_op = s->v;
   	SDL_CondSignal(s->cond);
   	SDL_UnlockMutex(s->mutex);

    return( value_after_op );
}


int lock_value (SyncLock * s)
{
    
    int         value_after_op;

   	SDL_LockMutex(s->mutex);
    value_after_op = s->v;
   	SDL_UnlockMutex(s->mutex);

    return (value_after_op);
}


void set_lock_value (SyncLock * s, SEMMODE mode)
{
   	SDL_LockMutex(s->mutex);
    if( mode == SEMPTY )
    {
        s->v = 0;
    }
    else
    {
        s->v = 1;
    }
   	SDL_UnlockMutex(s->mutex);
}



int bin_lock_init(SyncLock * s, SEMMODE mode)
{
	memset( (char *)s, 0x00, sizeof(SyncLock) );
	
    if( mode == SEMPTY )
    {
        s->v = 0;
    }
    else
    {
        s->v = 1;
    }
    
    s->mutex = SDL_CreateMutex();
    s->cond  = SDL_CreateCond();
    
    if( !s->mutex || !s->cond )
    {
    	fprintf(stderr, "mutex or condition variable create fail\n");
    	return -1;
    }

	return 0;    
}


int count_lock_init(SyncLock * s, SEMMODE mode, int init_value )
{
	memset( (char *)s, 0x00, sizeof(SyncLock) );
    if( mode == SEMPTY )
    {
        s->v = 0;
    }
    else
    {
        s->v = init_value;
    }
    
    s->mutex = SDL_CreateMutex();
    s->cond  = SDL_CreateCond();
    
    if( !s->mutex || !s->cond )
    {
    	fprintf(stderr, "mutex or condition variable create fail\n");
    	return -1;
    }

	return 0;    
}


int lock_destroy(SyncLock * s)
{
   	SDL_DestroyMutex(s->mutex);
   	SDL_DestroyCond(s->cond);
	
	memset( (char *)s, 0x00, sizeof(SyncLock) );

	return 0;    
}
