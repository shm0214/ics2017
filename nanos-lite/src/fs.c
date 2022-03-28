#include "fs.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}


int fs_open(const char *pathname, int flags, int mode) {
  for(int i = 0; i < NR_FILES; i++)
    if(strcmp(file_table[i].name, pathname) == 0)
      return i;
  assert(0);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  ssize_t size = file_table[fd].size;
  if(file_table[fd].open_offset + len > size)
    len = size - file_table[fd].open_offset;
  switch(fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
      return 0;
    case FD_DISPINFO:
      dispinfo_read(buf, file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    default:
      ramdisk_read(buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
      file_table[fd].open_offset += len;
      return len;
  }
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  ssize_t size = file_table[fd].size;
  switch(fd) {
    case FD_STDIN:
      return 0;
    case FD_STDOUT:
    case FD_STDERR:
      for (int i = 0; i < len; i++)
        _putc(((char*)buf)[i]);
      return len;
    case FD_FB:
      fb_write(buf, file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    default:
      if (file_table[fd].open_offset + len > size)
        len = size - file_table[fd].open_offset;
      ramdisk_write(buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
      file_table[fd].open_offset += len;
      return len;
  }
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  ssize_t size = file_table[fd].size;
  switch (whence) {
    case SEEK_CUR:
      if (offset >= 0)
        file_table[fd].open_offset = file_table[fd].open_offset + offset <= size ? file_table[fd].open_offset + offset : size;
      else 
        file_table[fd].open_offset = file_table[fd].open_offset + offset >= 0 ? file_table[fd].open_offset + offset : 0;
      break;
    case SEEK_SET:
      if (offset >= 0)
        file_table[fd].open_offset = offset <= size ? offset : size;
      else
        return -1;
    case SEEK_END:
      if (offset <= 0)
        file_table[fd].open_offset = size + offset >= 0 ? size + offset : 0;
      else
        return -1;
  }
  return file_table[fd].open_offset;
}

int fs_close(int fd) {
  return 0;
}