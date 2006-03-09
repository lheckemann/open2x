/*
 *  include/asm-s390/page.h
 *
 *  S390 version
 *    Copyright (C) 1999,2000 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *    Author(s): Hartmut Penner (hp@de.ibm.com)
 */

#ifndef _S390_PAGE_H
#define _S390_PAGE_H

#include <unistd.h>

#define PAGE_SIZE (getpagesize())
static __inline__ int getpageshift()
{
    int pagesize = getpagesize();
#if (__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))
    return (__builtin_clz(pagesize) ^ 31);
#else
    register int pageshift = -1;
    while (pagesize) { pagesize >>= 1; pageshift++; }
    return pageshift;
#endif
}
#define PAGE_SHIFT (getpageshift())
#define PAGE_MASK    (~(PAGE_SIZE-1))

#endif /* _S390_PAGE_H */
