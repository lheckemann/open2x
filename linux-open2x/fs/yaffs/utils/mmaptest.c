
// mmaptest.c
// Used to test mmap writing (ie yaffs_writepage)
//
// Written by James McKenzie
//

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

int
main (int argc, char *argv[])
{
  int fd;
  off_t size = 0;
  void *map;

  size = 6291456;

  (void) unlink ("testfile");

  fd = open ("testfile", O_RDWR | O_CREAT | O_TRUNC, 0666);

  if (fd < 0)
    {
      perror ("open");
      return -1;
    }

  if (lseek (fd, size, SEEK_SET) != size)
    {
      perror ("lseek");
      return -1;
    }

  if (write (fd, "", 1) != 1)
    {
      perror ("write");
      return -1;
    }

  size++;

  map = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED)
    {
      perror ("mmap");
      return -1;
    }

  errno = 0;

  memset (map, 1 + (*(unsigned char *) map), size);

  printf ("msync(map,8536,MS_SYNC) returns %d (errno=%d [%s])\n",
          msync (map, 8536, MS_SYNC), errno, strerror (errno));

  (void) munmap (map, size);

  return 0;
}
