/*
 * Read-Only Flash Translation Layer
 * (c) 2006 Brendan McCarthy <oddbot@gmail.com>
 * GPL Version 2 Only
 */

#ifndef _LINUX_ROFTL_H
#define _LINUX_ROFTL_H

/* 128 bytes */
static const char roftl_part_marker[128] =
"OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT"
"OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT"
"OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT";

static const char roftl_mt_marker[128] =
"OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT"
"OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT"
"OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" ;


typedef struct roftl_header_t
{   
    char      marker[128];
    char      version[4];
    u_int32_t data_length;
    u_int32_t block_size;
    u_int32_t block_size2;  // power of 2 of the block size, eg. 14 for 16kiB
    u_int32_t num_data_blocks; /* as in good ones, not including this header */
} roftl_header_t;


typedef struct roftl_mt_header_t
{
    char      marker[128];
    u_int32_t num_entries;
} roftl_mt_header_t;




#endif /* _LINUX_ROFTL_H */
