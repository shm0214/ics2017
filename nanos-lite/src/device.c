#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  bool down = 0;
  if (key & 0x8000) {
    key |= 0x8000;
    down = 1;
  }
  if (key) 
    sprintf(buf, "%s %s\n", down ? "kd" : "ku", keyname[key]);
  else
    sprintf(buf, "t %d\n", (uint32_t)_uptime());
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int x = offset / 4 / _screen.width;
  int y = offset / 4 % _screen.width;
  _draw_rect(buf, y, x, len / 4, 1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
