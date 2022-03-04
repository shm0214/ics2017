#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[100];
  uint32_t value;

  /* TODO: Add more members if necessary */


} WP;

void free_wp(int n);
WP* new_wp();
bool check_watchpoints();
void print_watchpoints();

#endif
