#include "common.h"
#include "syscall.h"

extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      SYSCALL_ARG1(r) = 1;
      break;
    case SYS_exit:
      _halt(a[1]);
      break;
    // case SYS_write:
    //   Log("%d", a[3]);
    //   if(a[1] == 1 || a[1] == 2)
    //     for(int i = 0; i < a[3]; i++) 
    //       _putc(((char*)a[2])[i]);
    //   SYSCALL_ARG1(r) = a[3];
    //   break;
    case SYS_brk:
      SYSCALL_ARG1(r) = 0;
      break;
    case SYS_lseek:
      SYSCALL_ARG1(r) = fs_lseek(a[1], a[2], a[3]);
      break;
    case SYS_read:
      SYSCALL_ARG1(r) = fs_read(a[1], (void*)a[2], a[3]);
      break;
    case SYS_write:
      SYSCALL_ARG1(r) = fs_write(a[1], (void*)a[2], a[3]);
      break;
    case SYS_open:
      SYSCALL_ARG1(r) = fs_open((char*)a[1], a[2], a[3]);
      break;
    case SYS_close:
      SYSCALL_ARG1(r) = fs_close(a[1]);
      break;
    default: 
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return r;
}
