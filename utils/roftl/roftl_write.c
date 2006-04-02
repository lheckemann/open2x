
/**
 * formats a partition in open2x format.
 *
 * This format is for read only filesystems.
 * After the header block is written, the filesystem data is
 * written linearly, block by block, skipping any bad blocks.
 * If OOB data is available, the logical address of each block
 * is stored there.
 * At the end of the filesystem data, another header followed 
 * by the list of logical block addresses is written.
 *
 * (c) Brendan McCarthy 2006
 * GPL
 * */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#ifdef NEW_HEADERS
#include <mtd/mtd-user.h>
#else
#include <linux/mtd/mtd.h>
#endif

#include <linux/mtd/ftl.h>

#include "roftl_common.h"


/* for testing */
static int random_badblocks = 0;

/* variables */
int part_fd;
mtd_info_t mtd;
int quiet = 0;
int have_oob = 0;
int block_size = 16384, page_size = 512;
struct mtd_oob_buf oob;
unsigned char oobbuf[64];
int num_badblocks;


static void print_size(u_int s)
{
    if ((s > 0x100000) && ((s % 0x100000) == 0))
	printf("%d mb", s / 0x100000);
    else if ((s > 0x400) && ((s % 0x400) == 0))
	printf("%d kb", s / 0x400);
    else
	printf("%d bytes", s);
}

static int random_badblock()
{
    if(random_badblocks)
        return (rand() % 10 == 0);
    else
        return 0;
}

static int build_header( open2x_part_header_t *header, struct stat *data_stat)
{
    int i = 1;
    u_int32_t block_size2 = 0;

    while ( (mtd.erasesize & i) == 0 )
    {
        i <<= 1;
        block_size2++;
    }

    printf("Block Size 2: %d\n", block_size2);
    
    memcpy(header->marker, Open2xPartMarker, 128);
    memcpy(header->version, "0002", 4);

    header->data_length  = TO_LE32(data_stat->st_size);
    header->block_size   = TO_LE32(mtd.erasesize);
    header->block_size2  = TO_LE32(block_size2);

    header->num_data_blocks = TO_LE32(data_stat->st_size >> block_size2);
    if (data_stat->st_size % mtd.erasesize  != 0)
    {
        header->num_data_blocks++;
    }

    return 0;
}

static int sanity_check(struct stat *data_stat)
{
    if (mtd.size < data_stat->st_size)
    {
        fprintf(stderr, "Data is too big for this partition\n");
        return 1;
    }
    else
    {
        printf("Data fits in partition\n");
    }

    if( mtd.oobblock != 0)
    {
        if (data_stat->st_size % mtd.oobblock != 0)
        {
            fprintf(stderr, "Data is not aligned to page size\n");
            return 1;
        }
        else
        {
            printf("Data is aligned to page size\n");
        }
    }

    return 0; /* all clear */
}


static int mark_bad_block()
{
    if(have_oob)
    {
        memset(oob.ptr, 0x00, mtd.oobsize);
        oob.length = mtd.oobsize;
        if (ioctl(part_fd, MEMWRITEOOB, &oob)  != 0 || oob.length != mtd.oobsize)
        {
            perror("Error marking block as bad");

            return 1;
        }
    }

    return 0;
}


static int write_important_data(
        u_int32_t *physical_block,
        void *data,
        int length)
{
    int i;
    int part_pos = 0;
    int done = 0;

    void *padded_data = NULL, *verify_data = NULL;
    
    int num_blocks = length / block_size;
    int read_offset = 0;

    int write_success = 0;

    if (length % block_size)
        num_blocks ++;

    printf("num_blocks: %d, block_size: %d\n", num_blocks, block_size);
    padded_data = malloc(num_blocks * block_size);
    verify_data = malloc(block_size);

    printf("allocted padded data at %p\n", padded_data);
    memset(padded_data, 0xFF, num_blocks * block_size);

    if (!padded_data || !verify_data)
    {
        fprintf(stderr, "unable to allocate write buffers for important data\n");
        return 1;
    }

    memcpy(padded_data, data, length);
    part_pos = (*physical_block) * block_size;
    while (!done && part_pos < mtd.size)
    {
        write_success = 0;
        lseek(part_fd, part_pos, SEEK_SET);

        if(have_oob && is_bad_block(part_fd, part_pos, &oob, &mtd))
        {
            goto next_block;
        }

        printf("Writing some data from %p offset %d\n", padded_data, read_offset);
        if (write(part_fd, padded_data + read_offset, block_size) != block_size)
        {
            perror("error writing important data");
            fprintf(stderr, "to block at %d\n", part_pos);
            goto next_block;
        }

        for(i = 0; i < 16; i++)
        {
            lseek(part_fd, part_pos, SEEK_SET);
            if (read(part_fd, verify_data, block_size) != block_size)
            {
                perror("error verifying importand data");
                fprintf(stderr, "on block at %d\n", part_pos);
                goto next_block;
            }

            if (memcmp(padded_data + read_offset, verify_data, block_size) != 0)
            {
                fprintf(stderr, "important data does not match at %d\n", part_pos);
                goto next_block;
            }
            else
            {
                putchar('.');
                fflush(stdout);
            }

            if (random_badblock())
            {
                fprintf(stderr, "random bad block at %d\n", part_pos);
                goto next_block;
            }
        }

        /* success */
        read_offset += block_size;
        done = (read_offset >= num_blocks * block_size);
        write_success = 1;

next_block:
        if(!write_success)
        {
            oob.start = part_pos;
            mark_bad_block();
        }
        (*physical_block) = (*physical_block) + 1;

        part_pos = (*physical_block) * block_size;
    }

    printf("\nwrite_important_data finished at block: %d\n", *physical_block);

    return !done;
}



static int format_partition(int data_fd)
{
    erase_info_t erase;
    
    struct stat data_stat;

    open2x_part_header_t  part_header;
    open2x_mt_header_t    mt_header;

    char *block, *block_verify;
    int erase_units;
    int i, write_success;
    int part_pos = 0, data_pos = 0, count = 0;
    u_int32_t  *mapping_table;
    u_int32_t logical_block, physical_block;
    u_int32_t num_data_blocks;

    int   mapping_table_data_length;
    void *mapping_table_data;

    int pretty_steps;

    num_badblocks = 0;
   
    memset(oobbuf, 0xFF, sizeof(oobbuf));

    if (ioctl(part_fd, MEMGETINFO, &mtd) != 0)
    {
        perror("get mtd into failed");
        return -1;
    }

    page_size = mtd.oobblock;
    if (page_size == 0)
    {
        page_size = 512;
        have_oob = 0;
    }
    else
    {
        have_oob = 1;
    }

    block_size = mtd.erasesize;
    block        = (char *) malloc(block_size);
    block_verify = (char *) malloc(block_size);

    
    if (!block || !block_verify)
    {
        perror("error allocating memory for block");
        return -1;
    }
    
    if(!quiet)
    {
        printf("Partition Size = ");
        print_size(mtd.size);
        printf(", erase_unit size = ");
        print_size(mtd.erasesize);
        printf(", page size = ");
        print_size(mtd.oobblock);
        printf(", OOB Availilable: %s", have_oob?"yes":"no");
        printf("\n");
    }


    /* set up the oob struct */
    oob.ptr = oobbuf;
    

    if (fstat(data_fd, &data_stat) != 0)
    {
        perror("error stat-ing data file");
        return -1;
    }

    if (sanity_check(&data_stat) != 0)
    {
        fprintf(stderr, "sanity check failed, exiting\n");
        return -1;
    }

    if (build_header(&part_header, &data_stat) != 0)
    {
        fprintf(stderr, "error building header, exiting\n");
        return -1;
    }

    num_data_blocks = FROM_LE32(part_header.num_data_blocks);
    printf("Allocating %d entries in mapping table\n", num_data_blocks);
    mapping_table = (u_int32_t*) malloc(num_data_blocks * sizeof(u_int32_t));

    if( !mapping_table)
    {
        fprintf(stderr, "Error allocating mapping table");
        return 1;
    }

    if (!quiet)
    {
        printf("Erasing Partition\n");
    }

    /* set the ecc format */
    if (have_oob)
    {
        if (ioctl (part_fd, MEMSETOOBSEL, &oobinfo) != 0)
        {
             perror ("MEMSETOOBSEL");
             return 1;
        }
    }

    /* erase the partition */
    erase_units = mtd.size / mtd.erasesize;
    
    erase.length = mtd.erasesize;
    erase.start = 0;
    printf("Erasing %d blocks: ", erase_units);
    pretty_steps = erase_units / 60 + 1;
    for(i = 0; i < erase_units; i++)
    {
        if(i % pretty_steps == 0)
        {
            putchar('*');
            fflush(stdout);
        }

        if (ioctl(part_fd, MEMERASE, &erase))
        {
            fprintf(stderr, "Erase of block %d failed\n", i);
            oob.start = erase.start;
            mark_bad_block();
            num_badblocks++;
        }
        erase.start += erase.length;
    }
    printf("\n");

    if (!quiet)
    {
        printf("Erase Complete with %d bad blocks\n", num_badblocks);
    }

    physical_block = 0;

    printf("Writing header:...\n");
    if (write_important_data(
                &physical_block,
                &part_header,
                sizeof(part_header)
                ) != 0)
    {
        fprintf(stderr, "unable to write header\n");
        return 1;
    }

    printf("Writing data, starting at block %d\n", physical_block);

    data_pos = 0;
    logical_block = 0;

    pretty_steps = data_stat.st_size / block_size / 60 + 1;

    while (data_pos != data_stat.st_size)
    {
        memset(block, 0xFF, block_size);
        count = read(data_fd, block, block_size);

        if (count < block_size)
        {
            if (data_pos < data_stat.st_size - block_size)
            {
                fprintf(stderr, "Premature end to data\n");
                return 1;
            }
        }

        if (count % page_size)
        {
            fprintf(stderr, "Data not aligned to page\n");
            return 1;
        }

        data_pos += count;

        if( (data_pos / block_size ) % pretty_steps == 0)
        {
            putchar('*');
            fflush(stdout);
        }

        write_success = 0;
        while(!write_success)
        {
            u_int32_t temp;

            part_pos = physical_block * block_size;

            /* seek to the next physical block */
            lseek(part_fd, part_pos, SEEK_SET);

            
            /* we have to write the oob data first so the ecc is corrected later */
            if(have_oob)
            {
                oob.start = part_pos;

                if (is_bad_block(part_fd, part_pos, &oob, &mtd))
                {
                    goto next_block;
                }

                oob.start = part_pos;
                temp = TO_LE32(logical_block);
                memset(oobbuf, 0xFF, sizeof(oobbuf));
                memcpy(oobbuf+8, &temp, sizeof(temp));
                oob.length = mtd.oobsize;
                
                if (ioctl(part_fd, MEMWRITEOOB, &oob) != 0 || oob.length != mtd.oobsize)
                {
                    perror("error writing oob: ");
                    fprintf(stderr, "error writing oob to block %x, length=%d\n", oob.start, oob.length);

                    goto next_block;
                }

                /* try and read the oob */
                if (ioctl(part_fd, MEMREADOOB, &oob) != 0 || oob.length != mtd.oobsize)
                {
                    perror("error reading back oob");
                    fprintf(stderr, "length: %d\n", oob.length);

                    goto next_block;
                }

                memcpy(&temp, oobbuf+8, sizeof(temp));
                if (FROM_LE32(temp) != logical_block)
                {
                    printf ("write mismatch, logical block %d read %d", logical_block, FROM_LE32(temp));

                    goto next_block;
                }

                //printf ("logical block %d oob written successfully\n", logical_block);
            }

            /* we're good, lets try to write the data now */
            if (write(part_fd, block, block_size) != block_size)
            {
                perror("Error writing block");
                fprintf(stderr, "error writing block at %x\n", part_pos);

                goto next_block;
            }

            /* read the data back to verify */
            for(i = 0; i < 16; i++)
            {
                lseek(part_fd, part_pos, SEEK_SET);
                if (read(part_fd, block_verify, block_size) != block_size)
                {
                    perror("Error reading back block");
                    fprintf(stderr, "error reading block at %d\n", part_pos);
                    goto next_block;
                }

                if (memcmp(block, block_verify, block_size) != 0)
                {
                    fprintf(stderr, "Error, contents do not verify at %d\n", part_pos);
                    goto next_block;
                }
            }


            if (random_badblock())
            {
                fprintf(stderr, "Random bad block at %d\n", part_pos);
                goto next_block;
            }
            
            /* success */
            printf("Set logical block %d to physical block %d\n", logical_block, physical_block);
            mapping_table[logical_block] = physical_block;

            logical_block++;

            write_success = 1;
next_block:
            if(!write_success)
            {
                oob.start = part_pos;
                mark_bad_block();
            }

            physical_block++;

            if(part_pos > mtd.size)
            {
                fprintf(stderr, "We've gone over the partition, bailing out");
                return 1;
            }
        }
    }

    mapping_table_data_length = sizeof(open2x_mt_header_t) + (part_header.num_data_blocks * sizeof(u_int32_t));
    mapping_table_data = malloc(mapping_table_data_length);

    if (!mapping_table_data)
    {
        fprintf(stderr, "unable to allocate buffer for mapping_table_data\n");
        return 1;
    }

    memcpy(mt_header.marker, Open2xMTMarker, 128);
    mt_header.num_entries = TO_LE32(part_header.num_data_blocks);
    memcpy(mapping_table_data, &mt_header, sizeof(open2x_mt_header_t));
    memcpy(mapping_table_data+sizeof(open2x_mt_header_t), mapping_table, part_header.num_data_blocks * sizeof(u_int32_t));

    printf("\nWriting mapping table:...\n");
    if (write_important_data(
                &physical_block, 
                mapping_table_data,
                mapping_table_data_length
                ) != 0)
    {
        fprintf(stderr, "unable to write mapping table\n");
        return 1;
    }


    printf("Partition formatted successfully\n");
    printf("Wrote %d bytes of data to %d blocks\n", data_pos, physical_block);
    printf("%d bad blocks detected\n", num_badblocks);

    return 0;
}

int main(int argc, char **argv)
{
    int data_fd;
    
    if (argc != 3)
    {
        printf("Usage: roftl_write: /dev/mtd/X image.dump\n");
        return 1;
    }

    printf("Using partition %s\n", argv[1]);
    part_fd = open(argv[1], O_RDWR);
    if (part_fd == -1)
    {
        perror("Error opening partition");
        return 1;
    }
    
    printf("Using data file %s\n", argv[2]);
    data_fd = open(argv[2], O_RDONLY);
    if (data_fd == -1)
    {
        perror("Error opening image file");
        return 1;
    }


    format_partition(data_fd);

    close(part_fd);
    close(data_fd);

    return 0;
}

