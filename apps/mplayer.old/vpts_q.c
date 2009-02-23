


#include "mmsp2_if.h"



#ifdef GDEBUG
extern FILE           *dbg;
	#define gprintf(x...) fprintf(dbg, x)
#else
	#define gprintf(x...)
#endif


#define QUEUE_SIZE  64 

static float    vpts_readq[QUEUE_SIZE];
int             vpts_readq_front = 0;
int             vpts_readq_rear = 0;

#define P_RQ_MSG() do {	\
	printf("%s vpts_readq_front(%d), vpts_readq_rear(%d)\n", __FUNCTION__, vpts_readq_front, vpts_readq_rear); \
}while(0)

inline int put_vpts_readq(float pts)
{
    if (((vpts_readq_rear + 1) % QUEUE_SIZE) == vpts_readq_front)
    {
        printf("vpts ring q overflow\n");
        
        return -1;
    }
#if 0
	#if defined(DEBUG_QUEUE)
   	P_RQ_MSG();
	#endif
#endif
    vpts_readq[vpts_readq_rear] = pts;
    vpts_readq_rear = (vpts_readq_rear + 1) % QUEUE_SIZE;
#if defined(DEBUG_QUEUE)
    P_RQ_MSG();
#endif
    return 1;
}

inline float get_vpts_readq(void)
{
    float ret;

    if(vpts_readq_front == vpts_readq_rear)
    {
        
        return -0.0;
    }

#if defined(DEBUG_QUEUE)
    P_RQ_MSG();
#endif
    ret = vpts_readq[vpts_readq_front];
    vpts_readq_front = (vpts_readq_front + 1) % QUEUE_SIZE;
    return ret;
}

void init_vpts_readq(void)
{
    vpts_readq_front 	= 0;
    vpts_readq_rear 	= 0;

    printf("VPTS ring Q initialize\n");
}


static float    nft_readq[QUEUE_SIZE];
int             nft_readq_front = 0;
int             nft_readq_rear = 0;

inline int put_nft_readq(float pts)
{
    if(((nft_readq_rear + 1) % QUEUE_SIZE) == nft_readq_front)
    {
        printf("nft ring q overflow\n");
        return -1;
    }
#if 0
	#if defined(DEBUG_QUEUE)
    P_RQ_MSG();
	#endif
#endif
    nft_readq[nft_readq_rear] = pts;
    nft_readq_rear = (nft_readq_rear + 1) % QUEUE_SIZE;
#if defined(DEBUG_QUEUE)
    P_RQ_MSG();
#endif
    return 1;
}

inline float get_nft_readq(void)
{
    float ret;

    if(nft_readq_front == nft_readq_rear)
    {
        
        return -0.0;
        
    }

#if defined(DEBUG_QUEUE)
    P_RQ_MSG();
#endif

    ret = nft_readq[nft_readq_front];
    nft_readq_front = (nft_readq_front + 1) % QUEUE_SIZE;

    return ret;
}

void init_nft_readq(void)
{
    nft_readq_front = 0;
    nft_readq_rear = 0;

    printf("NFT ring Q initialize\n");
}

