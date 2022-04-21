#include "common.h"

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_filesz(int fd);
extern int fs_close(int fd);
void* new_page(void);

#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
  // size_t size = get_ramdisk_size();
  // ramdisk_read((void *)DEFAULT_ENTRY, 0, size);
  int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);
  void* va = DEFAULT_ENTRY;
  void* pa;
  while (size > 0) {
    pa = new_page();
    _map(as, va, pa);
    fs_read(fd, pa, 4096);
    va += 4096;
    size -= 4096;
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
