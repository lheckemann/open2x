
#include "roftl_common.h"

#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>

int is_bad_block(int part_fd, int pos, struct mtd_oob_buf *oob, mtd_info_t *mtd)
{
    memset(oob->ptr, 0xFF, mtd->oobsize);
    oob->start  = pos;
    oob->length = mtd->oobsize;


    if (ioctl(part_fd, MEMREADOOB, oob) != 0 || oob->length != mtd->oobsize)
    {
        perror("error reading oob\n");
        fprintf(stderr, "at offset %d\n", pos);
        return -1;
    }
    else
    {
        char bbmarker = *((char*) oob->ptr + 5);
        if ( (bbmarker & 0xFF) != 0xFF)
        {
            fprintf(stderr, "found a bad block at %d: %0X%0X %0X%0X\n", 
                    pos, 
                    *(oob->ptr), 
                    *((oob->ptr) + 4),
                    *((oob->ptr) + 8),
                    *((oob->ptr) + 12)
                    );
            return -1;
        }
        else
        {
            return 0;
        }
    }
}

