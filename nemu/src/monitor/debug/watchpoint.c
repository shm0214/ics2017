#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  if (free_ == NULL) {
    printf("no more watchpoint!\n");
    assert(0);
  }
  WP* wp = free_;
  free_ = free_->next;
  if (!head) {
    wp->next = head;
    head = wp;
    return head;
  }
  WP* temp = head;
  for (; temp->next; temp = temp->next) {
    if (temp->NO < wp->NO && temp->next->NO > wp->NO) {
      wp->next = temp->next;
      temp->next = wp;
      return wp;
    }
  }
  temp->next = wp;
  wp->next = NULL;
  return wp;
}

void free_wp(int n) {
  WP* temp = head;
  if (head->NO == n){
    head = head->next;
    temp->next = NULL;
  } else 
    for(; temp->next; temp = temp->next) {
      if (temp->NO > n){
        printf("wrong number!\n");
        assert(0);
      }
      if (temp->next->NO == n) {
        WP* temp1 = temp->next;
        temp->next = temp->next->next;
        temp1->next = NULL;
        temp = temp1;
        break;
      }
    }
  temp->value = 0;
  memset(temp->expr, 0, sizeof(temp->expr));
  if (!free_) {
    temp->next = free_;
    free_ = temp;
  }
  WP* temp1 = free_;
  if (temp1->NO > temp->NO) {
    temp->next = temp1;
    free_ = temp;
    return;
  }
  for (; temp1->next; temp1 = temp1->next) {
    if (temp1->NO < temp->NO && temp1->next->NO > temp->NO) {
      temp->next = temp1->next;
      temp1->next = temp;
      return;
    }
  }
  temp1->next = temp;
  temp->next = NULL;
}


bool check_watchpoints() {
  WP* temp = head;
  bool flag = false;
  while(temp) {
    bool success = true;
    uint32_t value = expr(temp->expr, &success);
    if(!success) {
      printf("wrong expression!\n");
      continue;
    }
    if(value != temp->value) {
      flag = true;
      printf("watchpoint %d: %s\n", temp->NO, temp->expr);
      printf("Old Value = %d\n", temp->value);
      printf("New Value = %d\n", value);
      temp->value = value;
    }
    temp = temp->next;
  }
  return flag;
}


void print_watchpoints() {
  printf("Num\tWhat\tValue\n");
  WP* temp = head;
  while (temp) {
    printf("%d\t%s\t%d\n", temp->NO, temp->expr, temp->value);
    temp = temp->next;
  }
}