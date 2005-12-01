/*
 * linux/fs/fat/buffer.c
 *
 *
 */

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/msdos_fs.h>
#include <linux/fat_cvf.h>

#if 0
#  define PRINTK(x) printk x
#else
#  define PRINTK(x)
#endif

struct buffer_head *fat_bread(struct super_block *sb, int block)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	return MSDOS_SB(sb)->cvf_format->cvf_bread(sb,block);
}
struct buffer_head *fat_getblk(struct super_block *sb, int block)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	return MSDOS_SB(sb)->cvf_format->cvf_getblk(sb,block);
}
void fat_brelse (struct super_block *sb, struct buffer_head *bh)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	if (bh) 
		MSDOS_SB(sb)->cvf_format->cvf_brelse(sb,bh);
}
void fat_mark_buffer_dirty (
	struct super_block *sb,
	struct buffer_head *bh)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	MSDOS_SB(sb)->cvf_format->cvf_mark_buffer_dirty(sb,bh);
}
void fat_set_uptodate (
	struct super_block *sb,
	struct buffer_head *bh,
	int val)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	MSDOS_SB(sb)->cvf_format->cvf_set_uptodate(sb,bh,val);
}
int fat_is_uptodate(struct super_block *sb, struct buffer_head *bh)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	return MSDOS_SB(sb)->cvf_format->cvf_is_uptodate(sb,bh);
}
void fat_ll_rw_block (
	struct super_block *sb,
	int opr,
	int nbreq,
	struct buffer_head *bh[32])
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	MSDOS_SB(sb)->cvf_format->cvf_ll_rw_block(sb,opr,nbreq,bh);
}

struct buffer_head *default_fat_bread(struct super_block *sb, int block)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	return sb_bread(sb, block);
}

struct buffer_head *default_fat_getblk(struct super_block *sb, int block)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	return sb_getblk(sb, block);
}

void default_fat_brelse(struct super_block *sb, struct buffer_head *bh)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	brelse (bh);
}

void default_fat_mark_buffer_dirty (
	struct super_block *sb,
	struct buffer_head *bh)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	mark_buffer_dirty (bh);
}

void default_fat_set_uptodate (
	struct super_block *sb,
	struct buffer_head *bh,
	int val)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	mark_buffer_uptodate(bh, val);
}

int default_fat_is_uptodate (struct super_block *sb, struct buffer_head *bh)
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	return buffer_uptodate(bh);
}

void default_fat_ll_rw_block (
	struct super_block *sb,
	int opr,
	int nbreq,
	struct buffer_head *bh[32])
{
/* shkim */ //printk("##[%s:%d]->%s: \n", __FILE__, __LINE__, __FUNCTION__);
	ll_rw_block(opr,nbreq,bh);
}
