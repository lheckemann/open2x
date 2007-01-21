/*
 *  kernel file read operation
 */


#include "mmsp2_fread.h"

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

/* Context : User
 * Parameter : 
 *   filename : filename to open
 *   flags : 
 *     O_RDONLY, O_WRONLY, O_RDWR
 *     O_CREAT, O_EXCL, O_TRUNC, O_APPEND, O_NONBLOCK, O_SYNC, ...
 *   mode : file creation permission. 
 *     S_IRxxx S_IWxxx S_IXxxx (xxx = USR, GRP, OTH), S_IRWXx (x = U, G, O)
 * Return : 
 *   file pointer. if error, return NULL
 */
struct file *klib_fopen(const char *filename, int flags, int mode)
{
    struct file *filp = filp_open(filename, flags, mode);

    return (IS_ERR(filp)) ? NULL : filp;
}

/* Context : User
 * Parameter :
 *   filp : file pointer
 * Return : 
 */
void klib_fclose(struct file *filp)
{
    if (filp)
        fput(filp);
}

/* Context : User
 * Parameter : 
 *   filp : file pointer
 *   offset : 
 *   whence : SEEK_SET, SEEK_CUR
 * Comment : 
 *   do not support SEEK_END
 *   no boundary check (file position may exceed file size)
 */
#if 0 
loff_t generic_file_llseek(struct file *file, loff_t offset, int origin)
{
        long long retval;

        switch (origin) {
                case 2:
                        offset += file->f_dentry->d_inode->i_size;
                        break;
                case 1:
                        offset += file->f_pos;
        }
        retval = -EINVAL;
        if (offset>=0 && offset<=file->f_dentry->d_inode->i_sb->s_maxbytes) {
                if (offset != file->f_pos) {
                        file->f_pos = offset;
                        file->f_reada = 0;
                        file->f_version = ++event;
                }
                retval = offset;
        }
        return retval;
}

long filesize(FILE *stream)
{
   long curpos, length;

   curpos = ftell(stream);      
   fseek(stream, 0L, SEEK_END); 
   length = ftell(stream);      
   fseek(stream, curpos, SEEK_SET);
   return length;

}

#endif

/* Context : User
 * Parameter : 
 *   buf : buffer to read into
 *   len : number of bytes to read
 *   filp : file pointer
 * Return : 
 *   actually read number. 0 = EOF, negative = error
 */
int klib_fread(char *buf, int len, struct file *filp)
{
    int readlen;
    mm_segment_t oldfs;

    if (filp == NULL)
		return -ENOENT;
	if (filp->f_op->read == NULL) 
		return -ENOSYS;
	if (((filp->f_flags & O_ACCMODE) & O_RDONLY) != 0)
		return -EACCES;
			
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	readlen = filp->f_op->read(filp, buf, len, &filp->f_pos);
	set_fs(oldfs);

    return readlen;
}



long klib_fsize(struct file *filp)
{
	return filp->f_dentry->d_inode->i_size;
}

#if 0 // for test code

#define PA_ARM940_BASE	0x03000000
#define ARM940_MEM_SIZE	(1*1024*1024)
#define PHY_MEM_START (PA_ARM940_BASE+ARM940_MEM_SIZE)

#define TEMP_BUF_SIZE (1*1024*1024)

static int __init kfile_read_init(void)
{
    struct file *filep;
    long filesize;
    char *dptr;
    int readlen;

	printk("# Loading klib_file\n");

	printk("# klib_file : open for read\n");
   
    if ((filep = klib_fopen("./A940TFIRM.RAW", O_RDONLY, 0)) == NULL) {
		printk("Can't open file\n");
		return 1;
	}
	
	filesize = klib_fsize(filep);
	printk("filesize = %ld\n", filesize );

	#if 0 
	dptr = (char *)kmalloc(filesize, GFP_KERNEL);
	if( dptr == NULL )
	{
		printk("dptr kmalloc error\n");
		return -1;
	}
	#else
    dptr = ioremap_nocache(PHY_MEM_START, TEMP_BUF_SIZE);	
    #endif
	
	if( (readlen = klib_fread(dptr, filesize, filep) ) < 0 )
	{
		printk("data read error\n");
		return -1;
	}
	
	printk("read done\n");

    #if 0	
	kfree(dptr);
	#else
	iounmap(dptr);
	dptr = 0;	
	#endif
	
    klib_fclose(filep);

	printk("Unloading klib_file\n");

    #if 0 // kernel module test
    return -1; 
	#else
    return 0;
    #endif
}

static void __exit kfile_read_exit(void)
{
}

module_init(kfile_read_init);
module_exit(kfile_read_exit);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("kernel file operation routine");

#endif
