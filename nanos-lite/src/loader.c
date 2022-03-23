#include "common.h"

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);


#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  size_t size = get_ramdisk_size();
  ramdisk_read((void *)DEFAULT_ENTRY, 0, size);
  return (uintptr_t)DEFAULT_ENTRY;
}
