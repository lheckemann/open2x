
/**
 * Read only flash translation layer
 * for the open2x project
 * (c) Brendan McCarthy
 * GPL version 2 only
 *
 */


#include <linux/mtd/blktrans.h>
#include <linux/mtd/mtd.h>
#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/hdreg.h>
#include <linux/vmalloc.h>
#include <linux/blkpg.h>
#include <asm/uaccess.h>

#include <linux/mtd/roftl.h>

static int debug_level = 0;
MODULE_PARM(debug_level, "i");

#ifndef ROFTL_MAJOR
#define ROFTL_MAJOR 55
#endif

/* number of mtd devices and regions */
#define MAX_DEV     8
#define MAX_REGION  8

#define SECTOR_SIZE 512

#define PART_BITS 4


static struct nand_oobinfo oobinfo = {
    .useecc = MTD_NANDECC_AUTOPLACE,
    .eccbytes = 6,
    .eccpos = { 0, 1, 2, 3, 6, 7 },
    .oobfree = { { 8, 8} }
};

typedef struct partition_t {
    struct mtd_blktrans_dev  mbd;
    roftl_header_t           header;
    u_int32_t                *mapping_table;
    u_int32_t                header_block;
} partition_t;


static int locate_physical_block(partition_t *part, int block, loff_t *retval)
{
    loff_t offset = 0;
    u_char buf[64];
    int ret;
    u_int32_t block_address;
    int current_block;

    offset = (part->header_block + block) << part->header.block_size2;

    current_block = block + part->header_block + 1;
    offset = current_block << part->header.block_size2;

    while(offset < part->mbd.mtd->size)
    {
        ret = part->mbd.mtd->read_oob(part->mbd.mtd, offset, 16, &ret, buf);

        if (ret)
        {
            printk(KERN_WARNING "roftl: locate_physical_block: read_oob returned %d\n", ret);
            current_block++;
        }
        // check the bad block marker for 0s
        else if ( *(buf+5) != 0xFF )
        {
            printk(KERN_WARNING "roftl: hit a bad block at %d\n", current_block);
            current_block++;
        }
        else
        {
            memcpy(&block_address, buf + 8, sizeof(block_address));
            if (block_address == block)
            {
                printk(KERN_INFO "roftl: found block %d at %d\n", block, current_block);
                *retval = offset;
                return 0;
            }
            else if (block_address > block)
            {
                printk(KERN_WARNING "roftl: block_address %d > %d\n", block_address, block);
                return -EIO;
            }
            else if (block_address == 0)
            {
                printk(KERN_WARNING "roftl: got block address 0 ?!?\n");
                return -EIO;
            }
            else
            {
                int skip_blocks = block - block_address;
                printk(KERN_INFO "roftl: skipping %d blocks\n", skip_blocks);
                current_block += skip_blocks;
            }
        }

        offset = current_block << part->header.block_size2;
    } /* while offset < partition size */

    printk(KERN_WARNING "Fell off the end of the partition searching for block %d\n", block);
    return -EIO;
}


static int scan_header(partition_t *part)
{
    roftl_header_t header;
    loff_t offset, max_offset;
    int ret;
    int found = 0;

    part->header.data_length = 0;

    max_offset = part->mbd.mtd->size;

    // search the first megabyte for the header
    if (max_offset > (1 << 20))
        max_offset = (1 << 20);

    printk(KERN_NOTICE "roftl: scanning for header on the first %ld bytes of %s\n",
            (long) max_offset,
            part->mbd.mtd->name);

    for (offset = 0; 
            (offset + sizeof(header)) < max_offset;
            offset += part->mbd.mtd->erasesize ? : 0x2000)
    {
        ret = part->mbd.mtd->read_ecc(part->mbd.mtd, offset, sizeof(header), &ret,
                (unsigned char *) &header, NULL, &oobinfo);

        if (ret)
        {
            printk(KERN_NOTICE "roftl: scan header: read returned %d\n", ret);
            continue;
        }

        if (memcmp(header.marker, roftl_part_marker, 128) == 0)
        {
            found = 1;
            break;
        }
        else
        {
            break;
        }
    }

    if (!found)
    {
        printk( KERN_NOTICE "roftl: ROFTL header not found\n");
        return -ENOENT;
    }

    header.data_length = le32_to_cpu(header.data_length);
    header.block_size  = le32_to_cpu(header.block_size);
    header.block_size2 = le32_to_cpu(header.block_size2);
    header.num_data_blocks = le32_to_cpu(header.num_data_blocks);

    if (strncmp(header.version, "0001", 4) == 0)
    {
        printk (KERN_WARNING "roftl: old version detected, activating workarounds\n");
        header.block_size2 = 14; // was hard coded originally
    }

    /* check for corruption */
    if (header.data_length > part->mbd.mtd->size)
    {
        printk(KERN_NOTICE "roftl: data size is > partition size, header corrupt\n");
        return -1;
    }

    part->header = header;
    
    part->header_block = offset >> part->header.block_size2;

    printk(KERN_NOTICE "roftl: found %d byte ROFTL partition at %ld (block %d) on %s\n",
            part->header.data_length,
            (long) offset,
            part->header_block,
            part->mbd.mtd->name
          );

    return 0;
}

static int load_map(partition_t *part)
{
    roftl_mt_header_t mt_header;
    loff_t offset, max_offset;
    int ret;
    int found = 0;
    u_int32_t entries_per_block;
    int mt_offset, i;

    offset = part->header.data_length;

    // if the data is not block aligned, add another block
    if (offset & (part->header.block_size-1))
        offset += part->header.block_size;

    // add the number of blocks before the header
    offset += part->header_block << part->header.block_size2;

    printk(KERN_NOTICE "roftl: start was %ld\n", (long) offset);
    
    /* round off */
    offset = ((offset >> part->header.block_size2) << part->header.block_size2);

    max_offset = part->mbd.mtd->size;

    ret = locate_physical_block(part, part->header.num_data_blocks - 1, &offset);

    if (ret)
    {
        printk(KERN_WARNING "roftl: error skipping to header: %d\n", ret);
    }
    
    printk(KERN_NOTICE "roftl: scanning for map on %s, from %ld to %ld\n",
            part->mbd.mtd->name,
            (long) offset,
            (long) max_offset);

    /* search for the map */
    for (; offset + sizeof(mt_header) < max_offset;
            offset += part->mbd.mtd->erasesize ? : 0x2000)
    {
        ret = part->mbd.mtd->read_ecc(part->mbd.mtd, offset, sizeof(mt_header), &ret,
                (unsigned char *) &mt_header, NULL, &oobinfo);

        if (ret)
        {
            printk(KERN_NOTICE "roftl: load map: read returned %d\n", ret);
            continue;
        }

        if (memcmp(mt_header.marker, roftl_mt_marker, 128) == 0)
        {
            found = 1;
            break;
        }
    }


    if (!found)
    {
        printk( KERN_NOTICE "roftl: ROFTL mapping table not found\n");
        return -ENOENT;
    }

    mt_header.num_entries = le32_to_cpu(mt_header.num_entries);

    /* check for corruption */
    if (mt_header.num_entries != part->header.num_data_blocks)
    {
        printk(KERN_NOTICE "roftl: mt_header.num_entries != header.num_data_blocks\n");
        return -1;
    }

    
    part->mapping_table = vmalloc(sizeof(u_int32_t) * mt_header.num_entries);
    if (!part->mapping_table)
    {
        printk(KERN_WARNING "roftl: unable to allocate space for mapping table\n");
        return -1;
    }
    
    /* read the mapping table */
    offset += sizeof(mt_header);

    /* read the first block worth of mapping table entries */
    entries_per_block = (part->mbd.mtd->erasesize - sizeof(mt_header)) / sizeof(u_int32_t);

    if (mt_header.num_entries < entries_per_block)
    {
        entries_per_block = mt_header.num_entries;
    }
    mt_offset = 0;

    ret = part->mbd.mtd->read_ecc(part->mbd.mtd, offset, sizeof(u_int32_t) * entries_per_block,
            &ret, (unsigned char*) (part->mapping_table + mt_offset), NULL, &oobinfo);


    offset += entries_per_block * sizeof(u_int32_t);
    mt_offset = entries_per_block;
    entries_per_block = part->mbd.mtd->erasesize / sizeof(u_int32_t);
    
    while ((mt_offset < mt_header.num_entries) && (offset < max_offset))
    {
        ret = part->mbd.mtd->read_ecc(part->mbd.mtd, offset, sizeof(u_int32_t) * entries_per_block,
                &ret, (unsigned char*) (part->mapping_table + mt_offset), NULL, &oobinfo);

        if (! ret && (mt_offset + entries_per_block == mt_header.num_entries) )
        {
            /* all good */
            break;
        }

        offset += part->mbd.mtd->erasesize;

        if (ret)
        {
            printk(KERN_WARNING "roftl: load map: error reading mapping table: %d\n", ret);
            continue;
        }
        else
        {
            /* move to the next block, check to make sure we don't read past the end */
            mt_offset += entries_per_block;

            if (mt_header.num_entries < mt_offset + entries_per_block)
            {
                entries_per_block = mt_offset - mt_header.num_entries;
            }
        }
    }

    /* there was an error */
    if (offset >= max_offset)
    {
        printk(KERN_WARNING "roftl: load map: complete failure reading mapping table\n");
        vfree (part->mapping_table);
        part->mapping_table = NULL;
        return -1;
    }

    for(i = 0; i < mt_header.num_entries; i++)
    {
        part->mapping_table[i] = le32_to_cpu(part->mapping_table[i]);

        if (part->mapping_table[i] >= (part->mbd.mtd->size >> part->header.block_size2))
        {
            printk(KERN_WARNING "roftl: load map: mapping table entry goes beyond partition");
            vfree (part->mapping_table);
            part->mapping_table = NULL;
            return -1;
        }
    }

    printk(KERN_WARNING "roftl: successfully loaded mapping table for %s\n", part->mbd.mtd->name);
    
    return 0;
}

static int roftl_read(partition_t *part, caddr_t buffer, 
        u_long sector, u_long nblocks)
{
    u_int32_t offset, logical_block, physical_block, physical_address, page_offset;
    u_long i;
    size_t retlen;
    int ret;

    if (debug_level > 2)
        printk(KERN_WARNING "roftl: roftl_read: sector %ld, blocks %ld\n", sector, nblocks);
    
    for (i = 0; i < nblocks; i++)
    {
        offset = (sector + i) * SECTOR_SIZE;
        logical_block = (offset >> part->header.block_size2);
        page_offset = offset & (part->header.block_size-1);

        if (debug_level > 2)
            printk(KERN_NOTICE "roftl_read: offset: %d, logical block %d, page %d\n", 
                    offset, 
                    logical_block,
                    page_offset);

        if (logical_block <  0 || logical_block >= part->header.num_data_blocks)
        {
            printk(KERN_WARNING "roftl_read: logical block %d does not exist\n", logical_block);
            return -EIO;
        }

        physical_block   = part->mapping_table[logical_block];
        physical_address = (physical_block << part->header.block_size2) + page_offset;

        if (debug_level > 2)
            printk(KERN_INFO "roftl_read: reading from physical block %d address %d\n",
                    physical_block,
                    physical_address);

        ret = part->mbd.mtd->read_ecc(part->mbd.mtd, physical_address, SECTOR_SIZE,
                &retlen, (u_char *)buffer, NULL, &oobinfo);

        if (ret)
        {
            printk(KERN_WARNING "roftl_read: Error reading MTD device in roftl_read(): %d\n", ret);
            return ret;
        }

        buffer += SECTOR_SIZE;
    }

    return 0;
}

static int roftl_getgeo(struct mtd_blktrans_dev *dev, struct hd_geometry *geo)
{
    partition_t *part = (void *)dev;
    u_long sect;

    sect = le32_to_cpu(part->header.data_length) >> 9 ; // /SECTOR_SIZE;

    geo->heads = 1;
    geo->sectors = 8;
    geo->cylinders = sect >> 3;

    return 0;
}

static int roftl_readsect(struct mtd_blktrans_dev *dev, unsigned long block, char *buf)
{
    return roftl_read((void *)dev, buf, block, 1);
}

static void roftl_freepart(partition_t *part)
{
    if (part->mapping_table)
    {
        vfree(part->mapping_table);
        part->mapping_table = NULL;
    }
}

static void roftl_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
    partition_t *part;

    part = kmalloc(sizeof(partition_t), GFP_KERNEL);

    if (!part) {
        printk(KERN_WARNING "No memory to scan for ROFTL on %s\n", mtd->name);
        return;
    }

    memset(part, 0, sizeof(partition_t));

    part->mbd.mtd = mtd;

    if ((scan_header(part) == 0) &&
            (load_map(part) == 0))
    {
        printk(KERN_INFO "roftl: opening %d KiB ROFTL partition on %s\n", 
                le32_to_cpu(part->header.data_length) >> 10,
                part->mbd.mtd->name);

        part->mbd.size = le32_to_cpu(part->header.data_length) >> 9;
        part->mbd.blksize = SECTOR_SIZE;
        part->mbd.tr = tr;
        part->mbd.devnum = -1;

        if (add_mtd_blktrans_dev((void *)part))
            kfree(part);
    }
    else
    {
        kfree(part);
    }
}

static void roftl_remove_dev(struct mtd_blktrans_dev *dev)
{
    roftl_freepart( (partition_t*)dev);

    del_mtd_blktrans_dev(dev);
    kfree(dev);
}

struct mtd_blktrans_ops roftl_tr = {
    .name = "roftl",
    .major = ROFTL_MAJOR,
    .part_bits = PART_BITS,
    .readsect = roftl_readsect,
    .getgeo   = roftl_getgeo,
    .add_mtd = roftl_add_mtd,
    .remove_dev = roftl_remove_dev,
    .owner = THIS_MODULE,
};

int init_roftl(void)
{
    DEBUG(0, "ROFTL inited\n");

    return register_mtd_blktrans(&roftl_tr);
}

static void __exit cleanup_roftl(void)
{
    deregister_mtd_blktrans(&roftl_tr);
}

module_init(init_roftl);
module_exit(cleanup_roftl);

MODULE_AUTHOR ("Brendan McCarthy <oddbot@gmail.com>");
MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION ("Support for ROFTL for the gp2x");


