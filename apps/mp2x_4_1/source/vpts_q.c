#include "mmsp2_if.h"

#define QUEUE_SIZE  64 

static float    vpts_readq[QUEUE_SIZE];
static float    nft_readq[QUEUE_SIZE];
int             readq_front = 0;
int             readq_rear = 0;

inline int put_readq(float pts, float nft)
{
    if (((readq_rear + 1) % QUEUE_SIZE) == readq_front)
    {
        printf("ring q overflow\n");
        
        return 0;
    }

    vpts_readq[readq_rear] = pts;
	nft_readq[readq_rear] = nft;
    readq_rear = (readq_rear + 1) % QUEUE_SIZE;

	return 1;
}

inline int get_readq(float *pts, float *nft)
{
    if(readq_front == readq_rear)
    {
        printf("ring q underflow\n");
        
        return 0;
    }

    *pts = vpts_readq[readq_front];
    *nft = nft_readq[readq_front];
    readq_front = (readq_front + 1) % QUEUE_SIZE;

	return 1;
}

void init_readq(void)
{
    readq_front = 0;
    readq_rear = 0;

    printf("ring Q initialize\n");
}
