#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#undef NULL
#define NULL ((void*)0)

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   exit(); \
}

int
main(int argc, char *argv[])
{
  char *arg;

  int fd = open("tmp", O_WRONLY|O_CREATE);
  assert(fd != -1);
  printf(1, "open");  

  /* grow the heap a bit (move sz around) */
  assert((int)sbrk(4096 * 60) != -1);
  printf(1, "growth of heap");

  /* at zero */
  arg = (char*) 0x0;
  assert(write(fd, arg, 10) == -1);
  printf(1, "at zero");

  /* within null page */
  arg = (char*) 0x400;
  assert(write(fd, arg, 1024) == -1);
  printf(1, "null page");

  /* below code */
  arg = (char*) 0x1fff;
  assert(write(fd, arg, 1) == -1);
  printf(1, "below code");

  /* spanning null page and code bottom */
  assert(write(fd, arg, 2) == -1);
  printf(1, "below code"); 

  /* at code */
  arg = (char*) 0x2000;
  assert(write(fd, arg, 1) != -1);
  printf(1, "below code");
 
  /* within code/heap */
  arg = (char*) (((uint)sbrk(0) - 0x2000) / 2);
  assert(write(fd, arg, 40) != -1);
  printf(1, "within heap");

  /* at heap top */
  arg = (char*) ((uint)sbrk(0)-1);
  assert(write(fd, arg, 1) != -1);
  printf(1, "heap top");

  /* spanning heap top */
  assert(write(fd, arg, 2) == -1);
  printf(1, "spanning heap top");

  /* above heap top */
  arg = (char*) sbrk(0);
  assert(write(fd, arg, 1) == -1);
  printf(1, "above heap top");

  printf(1, "TEST PASSED\n");
  exit();
}

