/* $Id: suspend.h,v 1.1.8.1 2004/04/27 08:15:40 bushi Exp $ */

#ifndef __MTD_COMPAT_VERSION_H__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include_next <linux/suspend.h>
#endif

#endif /* __MTD_COMPAT_VERSION_H__ */
