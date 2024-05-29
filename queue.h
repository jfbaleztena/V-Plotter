#ifndef ___QUEUE_H__
#define ___QUEUE_H__
#include "vplotter.h"

typedef struct node{
    event_t event;
    struct node *next;
}node;

typedef struct queue{
    node *head;
    node *tail;
}linked_list;

linked_list event_queue;

void print_queue();
void enqueue(event_t new_event);
event_t dequeue();
bool is_queue_empty();

#endif