#ifndef LIBC_LIST_H_
#define LIBC_LIST_H_

#include <stddef.h>

/*
 * Link list utils
 */
struct list_node {
  struct list_node *next;
  struct list_node *prev;
};

#define LIST_HEAD(name) \
        struct list_node name = { &(name), &(name) }

static inline void list_init(struct list_node* head)
{
  head->prev = head->next = head;
}

static inline int list_empty(struct list_node *head)
{
  return head->next == head;
}

static inline void list_insert_back(struct list_node* head, struct list_node* elem)
{
  head->next->prev = elem;
  elem->next = head->next;
  elem->prev = head;
  head->next = elem;
}

static inline void list_insert_front(struct list_node* elem, struct list_node* head)
{
  head->prev->next = elem;
  elem->prev = head->prev;
  elem->next = head;
  head->prev = elem;
}

static inline void __list_del(struct list_node *prev, struct list_node *next)
{
  prev->next = next;
  next->prev = prev;
}

static inline void list_del(struct list_node* elem)
{
  __list_del(elem->prev, elem->next);
  elem->prev = NULL;
  elem->next = NULL;
}

static inline void list_del_init(struct list_node* elem)
{
  __list_del(elem->prev, elem->next);
  elem->prev = elem;
  elem->next = elem;
}

#define list_entry(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))
    
#define list_next_entry(head, type, member) \
      list_entry((head)->next, type, member)
#define list_prev_entry(head, type, member) \
      list_entry((head)->prev, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_continue(pos, head) \
    for (; pos != (head); pos = pos->next)

#endif