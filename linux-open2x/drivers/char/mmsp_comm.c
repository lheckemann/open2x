#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h> 
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/proto_gpio.h>
#include <asm/arch/mmsp20.h>		
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/slab.h>   
#include <linux/poll.h>   
#include <asm/delay.h>
#include <asm/ioctl.h>
#include <asm/unistd.h>

#define MMSP2_COMM_MAJOR 	89
#define MMSP2_COMM_NAME     ""



/* IOCTL  CMD*/

int MMSP2COMM_open(struct inode *inode, struct file *filp)
{
	return (0);         
}

int MMSP2COMM_release(struct inode *inode, struct file *filp)
{
	return (0);
}



int MMSP2COMM_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,unsigned long arg)
{
	return 0;			
}


struct file_operations MMSP2COMM_fops = {
	open:       MMSP2COMM_open,
	ioctl:      MMSP2COMM_ioctl,
	release:    MMSP2COMM_release,
};


static int __init MMSP2COMM_init(void)
{
	
	if( !register_chrdev( MMSP2_COMM_MAJOR, MMSP2_COMM_NAME, &MMSP2COMM_fops ) ) {
	
	}else{        
		printk(" unable to get major %d for %s \n", MMSP2_COMM_MAJOR, MMSP2_COMM_NAME );
		return -EBUSY;
	}
	
	return 0;
}

static void __exit MMSP2COMM_exit(void)
{  
    unregister_chrdev( MMSP2_COMM_MAJOR, MMSP2_COMM_NAME );
}

module_init(MMSP2COMM_init);
module_exit(MMSP2COMM_exit);