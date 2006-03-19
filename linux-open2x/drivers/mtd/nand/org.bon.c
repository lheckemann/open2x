#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>

#include <asm/uaccess.h>
#include <asm/errno.h>

#define BON_MAJOR 97

#define MAJOR_NR BON_MAJOR
#define DEVICE_NAME "bon"
#define DEVICE_REQUEST do_bon_request
#define DEVICE_NR(device) (device)
#define DEVICE_ON(device)
#define DEVICE_OFF(device)
#define DEVICE_NO_RANDOM
#include <linux/blk.h>

#include <linux/devfs_fs_kernel.h>

#define BON_DEBUG 0

#if BON_DEBUG
#define DPRINTK(x...) printk(x)
#else
#define DPRINTK(x...) (void)(0)
#endif


static int PARTITION_OFFSET  = (~0);

#define BONFS_OOB_ECCPOS0 8
#define BONFS_OOB_ECCPOS1 9
#define BONFS_OOB_ECCPOS2 10
#define BONFS_OOB_ECCPOS3 11
#define BONFS_OOB_ECCPOS4 12
#define BONFS_OOB_ECCPOS5 13

static struct nand_oobinfo bonfs_oob_8 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
};
static struct nand_oobinfo bonfs_oob_16 = {
	.useecc = MTD_NANDECC_PLACE,
	.eccbytes = 6,
	.eccpos = {BONFS_OOB_ECCPOS0, BONFS_OOB_ECCPOS1, BONFS_OOB_ECCPOS2,
		   BONFS_OOB_ECCPOS3, BONFS_OOB_ECCPOS4, BONFS_OOB_ECCPOS5},
	.oobfree = { {0,4}, {6,2}, {14,2}},
};
static struct nand_oobinfo bonfs_oob_64 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
};

#define MAX_RETRY	(2)
#define MAX_PART	(10)
#define MAX_OOB_BUF	(64)

typedef struct {
    unsigned long offset;
    unsigned long size;
    unsigned long flag;
    devfs_handle_t devfs_rw_handle;
    unsigned short *bad_blocks;
} partition_t;

static struct {
    struct mtd_info *mtd;
    int num_part;
    int num_mtd_part;
    devfs_handle_t devfs_dir_handle;
	struct nand_oobinfo *oobinfo;
    partition_t parts[MAX_PART];
} bon;

static const char BON_MAGIC[8] = {'M', 0, 0, 'I', 0, 'Z', 'I', 0};
static int bon_sizes[MAX_PART];
static int bon_blksizes[MAX_PART];

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
static struct proc_dir_entry *proc_bon;

static inline 
int bon_proc_info (char *buf, int i)
{
    partition_t *this = (partition_t *)&bon.parts[i];

    if (!this)
	return 0;

    return sprintf(buf, "bon%d: %8.8lx-%8.8lx (%8.8lx) %8.8lx\n", i, 
	this->offset, this->offset + this->size, this->size, this->flag);
}

static int 
bon_read_proc ( char *page, char **start, off_t off,int count 
		,int *eof, void *data_unused)
{
    int len, l, i;
    off_t   begin = 0;

    len = sprintf(page, "      position          size       flag\n");

    for (i=0; i< bon.num_part; i++) { 
	l = bon_proc_info(page + len, i);
	len += l;
	if (len+begin > off+count)
	    goto done;
	if (len+begin < off) {
	    begin += len;
	    len = 0;
	}
    }
    *eof = 1;

done:
    if (off >= len+begin)
	return 0;
    *start = page + (off-begin);
    return ((count < begin+len-off) ? count : begin+len-off);
}
#endif // CONFIG_PROC_FS

static int
bon_open(struct inode *inode, struct file *file)
{
    int minor = MINOR(inode->i_rdev);
    partition_t *part = &bon.parts[minor];
    if (minor >= bon.num_part || !part->size) return -ENODEV;

    get_mtd_device(bon.mtd, -1);

    return 0;
}

static int
bon_close(struct inode *inode, struct file *file)
{
    struct mtd_info *mtd;

    mtd = bon.mtd;

#if LINUX_VERSION_CODE != KERNEL_VERSION(2,4,18)
    invalidate_device(inode->i_rdev, 1);
#endif
    if (mtd->sync) mtd->sync(mtd);

    put_mtd_device(mtd);
    return 0;
}

static int
bon_ioctl(struct inode *inode, struct file *file,
    u_int cmd, u_long arg)
{
    int minor = MINOR(inode->i_rdev);
    partition_t *part = &bon.parts[minor];

    switch(cmd) {
	case BLKGETSIZE:
	    return put_user((part->size>>9), (long *)arg);
#ifdef BLKGETSIZE64
	case BLKGETSIZE64:
	    return put_user((u64)(part->size>>9), (long *)arg);
#endif
	case BLKFLSBUF:
	    if (!capable(CAP_SYS_ADMIN)) return -EACCES;
	    fsync_dev(inode->i_rdev);
	    invalidate_buffers(inode->i_rdev);
	    if (bon.mtd->sync) bon.mtd->sync(bon.mtd);
	    return 0;
	default:
	    return -ENOTTY;
    }
}

static struct block_device_operations bon_blk_fops = {
    owner: THIS_MODULE,
    open: bon_open,
    release: bon_close,
    ioctl: bon_ioctl
};

static int
do_read(partition_t *part, char *buf, unsigned long pos, size_t size)
{
    int ret, retlen;

    while(size > 0) {
	unsigned long block = pos / bon.mtd->erasesize;
	unsigned long start, start_in_block;
	size_t this_size;

	if (part->bad_blocks) {
	    unsigned short *bad = part->bad_blocks;
	    while(*bad++ <= block) {
		block++;
	    }
	}
	start_in_block = pos % bon.mtd->erasesize;
	start = block * bon.mtd->erasesize + start_in_block;
	this_size = bon.mtd->erasesize - start_in_block;
	if (this_size > size) this_size = size;

#if 0
	ret = MTD_READ(bon.mtd, part->offset + start, this_size, &retlen, buf);
#else
	ret = MTD_READECC(bon.mtd, part->offset + start, this_size, &retlen, buf, NULL, bon.oobinfo);
#endif
	if (ret != 0) {
	    printk("%s(): retlen = %d, ECC error ? - [%d]\n", 
			__FUNCTION__, retlen, ret);	
	    return ret;
	}
	if (this_size != retlen) {
	    printk("%s(): this_size = %d, retlen = %d\n", 
			__FUNCTION__, this_size, retlen);
	    return -EIO;
	}
	size -= this_size;
	buf += this_size;
	pos += this_size;
    }
    return 0;
}

static void
do_bon_request(request_queue_t *q)
{
    struct request *req;
    partition_t *part;
    int res;

    while (1) {
	INIT_REQUEST;
	req = CURRENT;
	spin_unlock_irq(&io_request_lock);
	res = 0;
	if (MINOR(req->rq_dev) >= bon.num_part) {
	    printk("bon: Unsupported devices\n");
	    goto end_req;
	}
	part = &bon.parts[MINOR(req->rq_dev)];
	if (req->current_nr_sectors << 9 > part->size) {
	    printk("bon: attempt to read past end of device!\n");
	    goto end_req;
	}
	switch(req->cmd) {
	    int err;
	    case READ:
		err = do_read(part, req->buffer, 
		    req->sector << 9, req->current_nr_sectors << 9);
		if (!err) res = 1;
		break;
	    case WRITE:
		break;
	}
end_req:
	spin_lock_irq(&io_request_lock);
	end_request(res);
    }
}

static int __read_partition_info(struct mtd_info *mtd, unsigned int **buf, int verb)
{
    unsigned long offset = PARTITION_OFFSET;
    int i;
    unsigned char oobbuf[MAX_OOB_BUF];
    unsigned int *s;
    unsigned char *buf_p = (unsigned char*)(*buf);
    int ret, retlen;
    int retry_count = MAX_RETRY;
	struct nand_chip *this = mtd->priv;

    if (offset > mtd->size - mtd->erasesize) 
	offset = mtd->size - mtd->erasesize;

#if 1
	DPRINTK("%s(): OOBSIZE = %d\n", __FUNCTION__, mtd->oobsize);
	switch (mtd->oobsize) {
	case 8:
		bon.oobinfo = &bonfs_oob_8;
		break;
	case 16:
		bon.oobinfo = &bonfs_oob_16;
		break;
	case 64:
		bon.oobinfo = &bonfs_oob_64;
		break;
	default:
		printk("BON does not yet know how to handle ECC\n");
		return -EINVAL;
	}
#endif

    while(retry_count-- > 0) {
#if 0
	if (MTD_READOOB(mtd, offset, 8, &retlen, oobbuf) < 0) {
		int l;
		for (l=0;l<8;l++) {
			printk(" 0x%02x", oobbuf[l]);
		}
		printk("\n");
	    goto prev_block;
	}
	if (oobbuf[5] != 0xff) {
	    goto prev_block;
	}
	if (MTD_READ(mtd, offset, 512, &retlen, buf_p) < 0) {
	    goto prev_block;
	}
#else
	if (MTD_READOOB(mtd, offset, 16, &retlen, oobbuf) < 0) {
	    goto prev_block;
	}
	if (oobbuf[5] != 0xff) {
	    goto prev_block;
	}

	ret = MTD_READECC(mtd, offset, 512, &retlen, buf_p, &oobbuf, bon.oobinfo	);
	if (ret < 0) {
		goto prev_block;
	}
#endif

	if (strncmp(buf_p, BON_MAGIC, 8) == 0) break;
	printk("bon:cannot find partition table\n");
	return -1;
prev_block:
        offset -= mtd->erasesize;
    }

    if (retry_count <= 0) {
	printk("bon:cannot find partition table\n");
	return -1;
    }

	mtd->oobinfo.useecc = MTD_NANDECC_AUTOPLACE;
#if 0
	this->autooob = bon.oobinfo;
#endif

    s = (unsigned int *)(buf_p + 8);
    bon.num_part = min_t(unsigned long, *s++, MAX_PART);

    for(i=0;i < bon.num_part; i++) {
	bon.parts[i].offset = *s++;
	bon.parts[i].size = *s++;
	bon.parts[i].flag = *s++;

	if (verb)
	    printk("bon%d: %8.8lx-%8.8lx (%8.8lx) %8.8lx\n", i, 
		    bon.parts[i].offset, 
		    bon.parts[i].offset + bon.parts[i].size, 
		    bon.parts[i].size, 
		    bon.parts[i].flag);
    }
    (*buf) = s;

    return bon.num_part;
}

static int
read_partition_info(struct mtd_info *mtd)
{
    int i;
    char buf[512];
    unsigned int *s = (unsigned int*)&buf[0];
    int k;

    if (__read_partition_info(mtd, &s, 1) < 0) return -1;

    bon.mtd = mtd;

    bon.devfs_dir_handle = devfs_mk_dir(NULL, "bon", NULL);

    for(i=0;i < bon.num_part; i++) {
	char name[8];
	if (bon.parts[i].flag & 0x01) {
	    /*
	     * do nothing
	     */
	} else {
	    sprintf(name, "%d", i);
	    bon.parts[i].devfs_rw_handle = devfs_register(bon.devfs_dir_handle,
		name, DEVFS_FL_DEFAULT, BON_MAJOR, i, 
		S_IFBLK | S_IRUGO, // | S_IWUGO, /* forbid writing */
		&bon_blk_fops, NULL);
	}
    }

    for(i=0;i<bon.num_part;i++) {
	unsigned int num_bad_block = *(s++) & 0xffff;
	if (num_bad_block == 0) continue;
	bon.parts[i].bad_blocks = kmalloc((1+num_bad_block) * sizeof(unsigned short), GFP_KERNEL);
	for(k=0;k<num_bad_block;k++) {
	    bon.parts[i].bad_blocks[k] = *s++;
	}
	bon.parts[i].bad_blocks[k] = ~0;
    }

    return 0;
}

static char *bon_name = "bon";
static char *mtd_name = "mtd";

int bon_check_mtd(struct mtd_info *mtd, struct mtd_partition **mtd_table)
{
    int i;
    char buf[2048];
    unsigned int *s = (unsigned int*)&buf[0];
    int bon_block_num = 0;
    int mtd_block_num = 0;
    struct mtd_partition *tmp = NULL;
	struct nand_chip *this = mtd->priv;

    bon_block_num = __read_partition_info(mtd, &s, 0);
    if (bon_block_num < 0) return 0;

    for (i=0; i< bon_block_num; i++)
	if (bon.parts[i].flag & 0x01) mtd_block_num++;

	if (*mtd_table) kfree(*mtd_table);
	(*mtd_table) = NULL;

    if (mtd_block_num) {
	(*mtd_table) = kmalloc(sizeof(u_char) *
	    sizeof(struct mtd_partition) * (mtd_block_num + 1),
	    GFP_KERNEL);
	if (!(*mtd_table)) return 0;
	memset(*mtd_table, 0, sizeof(struct mtd_partition) * (mtd_block_num + 1));

#if 1
	mtd->oobinfo.useecc = MTD_NANDECC_AUTOPLACE;
	this->autooob = bon.oobinfo;
#endif

	tmp = (*mtd_table);
	tmp->offset = 0;
	tmp->size = mtd->size;
	tmp->name = bon_name;
	tmp->oobsel = bon.oobinfo;
	tmp++;
	for (i=0;i<bon.num_part; i++) {
	    if (bon.parts[i].flag & 0x01) {
		tmp->offset = bon.parts[i].offset;	
		tmp->size = bon.parts[i].size;	
		tmp->name = mtd_name;
		tmp->oobsel = bon.oobinfo;
		tmp++;
	    }
	}
    }
    return mtd_block_num + 1;
}

static void
bon_notify_add(struct mtd_info *mtd)
{
    int i;

    if (!mtd->read_oob) return; 
    if (bon.num_part) return;
    if (read_partition_info(mtd)) return;
    for(i=0;i<bon.num_part;i++) {
	bon_sizes[i] = bon.parts[i].size/1024;
	bon_blksizes[i] = mtd->erasesize;
	if (bon_blksizes[i] > PAGE_SIZE) bon_blksizes[i] = PAGE_SIZE;
    }
}

static void
bon_notify_remove(struct mtd_info *mtd)
{
    int i;
    if (!bon.num_part || bon.mtd != mtd) return;
    devfs_unregister(bon.devfs_dir_handle);
    for(i=0;i<bon.num_part;i++) {
	devfs_unregister(bon.parts[i].devfs_rw_handle);
    }
    memset(&bon, 0, sizeof(bon));
}

static struct mtd_notifier bon_notifier = {
    add: bon_notify_add,
    remove: bon_notify_remove
};

static int
init_bon(void)
{
    int i;

    memset(&bon, 0, sizeof(bon));

    if (devfs_register_blkdev(BON_MAJOR, "bon", &bon_blk_fops)) {
	printk(KERN_WARNING "bon: unable to allocate major device num\n");
	return -EAGAIN;
    }

    for(i=0;i<MAX_PART;i++) {
        bon_blksizes[i] = 1024;
	bon_sizes[i] = 0;
    }

    blksize_size[BON_MAJOR] = bon_blksizes;
    blk_size[BON_MAJOR] = bon_sizes;

    blk_init_queue(BLK_DEFAULT_QUEUE(BON_MAJOR), &do_bon_request);
    register_mtd_user(&bon_notifier);

#ifdef CONFIG_PROC_FS
    if ((proc_bon = create_proc_entry( "bon", 0, 0 )))
	proc_bon->read_proc = bon_read_proc;
#endif

	printk("bonfs registered\n");

    return 0;
}

static void __exit cleanup_bon(void)
{
    unregister_mtd_user(&bon_notifier);
    devfs_unregister_blkdev(BON_MAJOR, "bon");
    blk_cleanup_queue(BLK_DEFAULT_QUEUE(BON_MAJOR));
    blksize_size[BON_MAJOR] = NULL;

#ifdef CONFIG_PROC_FS
    if (proc_bon)
	remove_proc_entry( "bon", 0);
#endif
}

#ifdef MODULE
MODULE_PARM(PARTITION_OFFSET, "i");
#else
int __init part_setup(char *options)
{
    if (!options || !*options) return 0;
    PARTITION_OFFSET = simple_strtoul(options, &options, 0);
    if (*options == 'k' || *options == 'K') {
	PARTITION_OFFSET *= 1024;
    } else if (*options == 'm' || *options == 'M') {
	PARTITION_OFFSET *= 1024;
    }
    return 0;
}
__setup("nand_part_offset=", part_setup);
#endif

module_init(init_bon);
module_exit(cleanup_bon);

//EXPORT_SYMBOL(bonfs_oob);
EXPORT_SYMBOL(bon_check_mtd);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Simple Block Device for Nand Flash");

