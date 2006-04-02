
#ifndef _GP2X_STUFF
#define _GP2X_STUFF


#include <sys/types.h>
#include <byteswap.h>
#include <endian.h>

#ifdef NEW_HEADERS
#include <mtd/mtd-user.h>
#else
#include <linux/mtd/mtd.h>
#endif


#if __BYTE_ORDER == __LITTLE_ENDIAN
# define TO_LE32(x) (x)
# define TO_LE16(x) (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
# define TO_LE32(x) (bswap_32(x))
# define TO_LE16(x) (bswap_16(x))
#else
# error cannot detect endianess
#endif

#define FROM_LE32(x) TO_LE32(x)
#define FROM_LE16(x) TO_LE16(x)


/* 128 bytes */
static const char Open2xPartMarker[128] =
"OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT"
"OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT"
"OPEN2XPT" "OPEN2XPT" "OPEN2XPT" "OPEN2XPT";

static const char Open2xMTMarker[128] =
"OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT"
"OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT"
"OPEN2XMT" "OPEN2XMT" "OPEN2XMT" "OPEN2XMT" ;




typedef struct open2x_part_header_t
{   
    char      marker[128];
    char      version[4];
    u_int32_t data_length;
    u_int32_t block_size;
    u_int32_t block_size2;
    u_int32_t num_data_blocks; /* as in good ones, not including this header */
} open2x_part_header_t;


typedef struct open2x_mt_header_t
{
    char      marker[128];
    u_int32_t num_entries;
} open2x_mt_header_t;


static struct nand_oobinfo oobinfo = {
    .useecc = MTD_NANDECC_AUTOPLACE,
    .eccbytes = 6,
    .eccpos = { 0, 1, 2, 3, 6, 7 },
    .oobfree = { { 8, 8} }
};

int is_bad_block(int part_fd, int pos, struct mtd_oob_buf *oob, mtd_info_t *mtd);



#endif
