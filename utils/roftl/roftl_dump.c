
/*
 * Dumps the contents of an open2x format partition
 *
 * (c) Brendan McCarthy 2006
 * GPL
 *
 */

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

#include "roftl_common.h"


/* variables */
int part_fd;
mtd_info_t mtd;
int quiet = 0;
int have_oob = 0;
int block_size = 16384;
int page_size = 512;
struct mtd_oob_buf oob;
unsigned char oobbuf[64];
int start_block = 0;

u_int32_t mapping_table_entries;
u_int32_t *mapping_table;


static void print_size(u_int s)
{
    if ((s > 0x100000) && ((s % 0x100000) == 0))
	printf("%d mb", s / 0x100000);
    else if ((s > 0x400) && ((s % 0x400) == 0))
	printf("%d kb", s / 0x400);
    else
	printf("%d bytes", s);
}

/** if scan_blocks == -1 then go the the end of the partition */
static int scan_for_marker( int start_block, const char *marker, int marker_len, int scan_blocks)
{
    int found = 0;
    int blocks = 0;
    int bad_block = 0;

    int part_pos = start_block * block_size;

    char *buffer = malloc(marker_len);
    if(!buffer)
    {
        fprintf(stderr, "scan_for_marker: Error allocating buffer\n");
        return -1;
    }

    while( !found && part_pos < mtd.size && (scan_blocks < 0 || blocks <= scan_blocks) )
    {
        lseek(part_fd, part_pos, SEEK_SET);
        bad_block = 0;
        if( read(part_fd, buffer, marker_len) != marker_len)
        {
            perror("scan_for_marker: cannot read data\n");
            bad_block = 1;
        }
        else
        {
            if (memcmp(buffer, marker, marker_len) == 0)
            {
                found = 1;
                break;
            }
        }

        if(!bad_block)
        {
            blocks++;
        }

        start_block++;
        part_pos = start_block * block_size;
    }

    free(buffer);

    return found ? start_block : -1;
}

static int read_good_block(int part_fd, int block, char *buffer)
{
    int done = 0;
    int num_blocks = mtd.size / block_size;

    while (!done && block < num_blocks)
    {
        lseek(part_fd, start_block * block_size, SEEK_SET);
        if (read(part_fd, buffer, block_size) != block_size)
        {
            perror("read_good_block: error reading block: ");
            fprintf(stderr, "at block %d\n", block);
            block++;
        }
        else
        {
            done = 1;
        }
    }

    return  done ? block : -1;
}

static int locate_block_by_oob(int part_fd, int header_block, int search_block)
{
    int done = 0;
    int num_blocks = mtd.size / block_size;
    int seek_dist;
    u_int32_t logical_block;

    u_int32_t block = search_block + header_block + 1;

    while (!done && block < num_blocks)
    {
        if (is_bad_block(part_fd, block * block_size, &oob, &mtd))
        {
            block++;
        }
        else
        {
            memcpy(&logical_block, oob.ptr + 8, sizeof(logical_block));
            logical_block = FROM_LE32(logical_block);

            printf("searching for block %d, got %d\n", search_block, logical_block);

            if (logical_block == search_block)
            {
                done = 1;
            }
            else if (logical_block > search_block)
            {
                fprintf(stderr, "logical block (%d) > search_block (%d), aborting\n", logical_block, search_block);
                return -1;
            }
            else
            {
                seek_dist = search_block - logical_block;
                printf("skipped forward %d blocks\n", seek_dist);
                block += seek_dist;
            }
        }
    }

    return done ? block : -1;
}

static int read_header(open2x_part_header_t *header)
{
    int header_block = scan_for_marker(0, Open2xPartMarker, 128, 0);

    if(header_block < 0)
    {
        fprintf(stderr, "unable to find partition marker\n" );
        return -1;
    }

    lseek(part_fd, header_block * block_size, SEEK_SET);
    
    if( read(part_fd, header, sizeof(open2x_part_header_t)) != sizeof(open2x_part_header_t))
    {
        perror("error reading header\n");
        return -1;
    }

    header->data_length     = FROM_LE32(header->data_length);
    header->block_size      = FROM_LE32(header->block_size);
    header->block_size2     = FROM_LE32(header->block_size2);
    header->num_data_blocks = FROM_LE32(header->num_data_blocks);

    return header_block;
}

static int read_mapping_table(open2x_part_header_t *header, u_int32_t *table)
{
    open2x_mt_header_t mt_header;
    int mt_block = start_block + header->num_data_blocks + 1;
    int i = 0;

    printf("starting search for mapping table at block %d\n", mt_block);
    mt_block = scan_for_marker(mt_block, Open2xMTMarker, 128, -1);

    if (mt_block == -1)
    {
        fprintf(stderr, "could not locate mapping table\n");
        return -1;
    }

    printf("Found mapping table header at block %d\n", mt_block);

    lseek(part_fd, mt_block * block_size, SEEK_SET);

    if (read(part_fd, &mt_header, sizeof(mt_header)) != sizeof(mt_header))
    {
        perror("error reading mapping table header");
        return -1;
    }

    mt_header.num_entries = FROM_LE32(mt_header.num_entries);

    if(header->num_data_blocks != mt_header.num_entries)
    {
        fprintf(stderr, "mapping table doesn't match header\n");
        return -1;
    }

    if (read(part_fd, table, mt_header.num_entries * sizeof(u_int32_t)) 
            != mt_header.num_entries * sizeof(u_int32_t))
    {
        perror("error reading mapping table entries\n");
        return -1;
    }

    for(i = 0; i < mt_header.num_entries; i++)
    {
        table[i] = FROM_LE32(table[i]);
        if(table[i] >= mtd.size / block_size)
        {
            fprintf(stderr, "mapping table entry goes beyond partition");
            return -1;
        }
    }

    return 0;
}

static int sanity_check(open2x_part_header_t *header)
{
    if (mtd.erasesize != header->block_size)
    {
        fprintf(stderr, "block size mismatch\n");
        return 1;
    }

    if (header->num_data_blocks * block_size < header->data_length)
    {
        fprintf(stderr, "corrupt header\n");
        return 1;
    }

    if (header->num_data_blocks * block_size > mtd.size)
    {
        fprintf(stderr, "corrupt header\n");
        return 1;
    }

    if (header->data_length % page_size != 0)
    {
        fprintf(stderr, "data length not aligned to page_size\n");
        return 1;
    }

    return 0; /* all clear */
}


static int dump_partition(int data_fd)
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

    int pretty_steps;
    
    char *buffer;
    
    start_block = 0;



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
        print_size(page_size);
        printf(", OOB Availilable: %s", have_oob?"yes":"no");
        printf("\n");
    }

    /* set up the oob struct */
    oob.ptr = oobbuf;
 
    /* set the ecc format */
    if (have_oob)
    {
        if (ioctl (part_fd, MEMSETOOBSEL, &oobinfo) != 0)
        {
             perror ("MEMSETOOBSEL");
             return 1;
        }
    }   

    /* read the header */
    start_block = read_header(&part_header);

    if (start_block < 0)
    {
        fprintf(stderr, "unable to find header, exiting\n");
        return -1;
    }
    
    if (sanity_check(&part_header) != 0)
    {
        fprintf(stderr, "sanity check failed, exiting\n");
        return -1;
    }

    printf("Allocating %d entries in mapping table\n", part_header.num_data_blocks);
    mapping_table = (u_int32_t*) malloc(part_header.num_data_blocks * sizeof(u_int32_t));

    if( !mapping_table)
    {
        fprintf(stderr, "Error allocating mapping table");
        return 1;
    }

    printf("Searching for mapping table\n");
    
    if (read_mapping_table(&part_header, mapping_table) == -1)
    {
        perror("unable to read mapping table\n");
        if(!have_oob)
        {
            fprintf(stderr, "without the mapping table or oob data, we are fucked, good day\n");
            return -1;
        }
    }

    printf("dumping data\n");

    buffer = malloc(block_size);
    if (!buffer)
    {
        fprintf(stderr, "error allocating block buffer\n");
        return -1;
    }

    for(i = 0; i < part_header.num_data_blocks; i++)
    {
        int real_block = mapping_table[i];
        int write_len = block_size;

        if (have_oob)
        {
            int my_block = locate_block_by_oob(part_fd, start_block, i);

            if(my_block != real_block)
            {
                fprintf(stderr, "methods do not match addresses at %d: %d != %d\n", i, my_block, real_block);
            }
            else
            {
                real_block = my_block;
            }
        }

        if (i == part_header.num_data_blocks - 1)
        {
            write_len = part_header.data_length % block_size;
        }

        //putchar('*');
        fflush(stdout);
        
        lseek(part_fd, real_block * block_size, SEEK_SET);
        if( read(part_fd, buffer, block_size) != block_size)
        {
            perror("error reading block");
            fprintf(stderr, "error reading data from logical block %d, physical block %d\n", i, real_block);
            return -1;
        }

        write(data_fd, buffer, write_len);
    }

    printf("data dumped\n");

    return 0;
}

int main(int argc, char **argv)
{
    int data_fd;
    
    if (argc != 3)
    {
        printf("Usage: roftl_dump: /dev/mtd/X image.dump\n");
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
    data_fd = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC);
    if (data_fd == -1)
    {
        perror("Error opening image file");
        return 1;
    }


    dump_partition(data_fd);

    close(part_fd);
    close(data_fd);

    return 0;
}

