#ifndef _TYPES_H_
#define _TYPES_H_

// Type definitions

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

typedef struct __lock_t{
  uint lock;
}lock_t;

typedef struct __cond_t{
  lock_t cond_lock;
  uint cond;
}cond_t;

typedef struct __sem_t{
  lock_t sem_lock;
  cond_t sem_cond;
  uint count;
}sem_t;

#ifndef NULL
#define NULL (0)
#endif

#endif //_TYPES_H_