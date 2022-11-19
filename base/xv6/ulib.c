#include "types.h"
#include "mmu.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

int
thread_create(void (*start_routine) (void *, void *), void *arg1, void *arg2)
{
    // get size of space and header
    uint h = sizeof(uint);
    uint size = 2 * (uint)PGSIZE + h;

    // create the memory and make sure its not zero
    uint stack_space = (uint) malloc(size);
    if(stack_space == 0){
            return -1;
    }

    // create page to build the stack
    uint page = stack_space + h;
    uint final_stack = PGROUNDUP(page);
    uint *top = (uint *) (final_stack - h);
    *top = stack_space;

    // call clone to create the thread, make sure -1 isn't returned
    int create = clone(start_routine, arg1, arg2, (void *) final_stack);
    if(create == -1){
            free((void *)stack_space);
    }

    // return created thread pid
    return create;
}
int
thread_join()
{
        // initialize stack and call join
        void *space;
        int new_thread = join(&space);

        // check if join returns -1
        if(new_thread == -1){
                return -1;
        }

        // set address space
        uint *top = ((uint *) space) -1;
        void *block = (void *) *top;

        free(block);
        return new_thread;
}

void
lock_acquire(lock_t *lock)
{
        // spin and wait for lock to be free
        while(xchg(&lock->flag, 1) != 0){
                ;
        }

        // acquire the lock
        if(lock->flag == 0){
                lock->flag = 1;
        }


}

void
lock_release(lock_t *lock)
{
        // release lock
        if(lock->flag == 1){
                lock->flag = 0;
        }
}

void
lock_init(lock_t *lock)
{
        // set lock initially to zero
        lock->flag = 0;
}
char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}
void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}
                                 
