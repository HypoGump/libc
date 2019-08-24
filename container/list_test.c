#include "list.h"
#include <stdlib.h>
#include <stdio.h>

struct tlist {
  struct list_node node;
  int val;
};

LIST_HEAD(thead);

int main()
{
  struct tlist* node0 = (struct tlist*)malloc(sizeof(struct tlist));
  node0->val = 0;
  list_insert_back(&thead, &node0->node);
  
  struct tlist* node1 = (struct tlist*)malloc(sizeof(struct tlist));
  node1->val = 1;
  list_insert_back(&thead, &node1->node);
  
  struct tlist* node2 = (struct tlist*)malloc(sizeof(struct tlist));
  node2->val = 2;
  list_insert_back(&thead, &node2->node);
  
  struct tlist* node3 = (struct tlist*)malloc(sizeof(struct tlist));
  node3->val = 3;
  list_insert_back(&thead, &node3->node);
  
  
  struct list_node *pos;
  list_for_each(pos, &thead) {
    struct tlist* entry = list_entry(pos, struct tlist, node);
    printf("%d ", entry->val);
  }
  
  printf("\n");
}