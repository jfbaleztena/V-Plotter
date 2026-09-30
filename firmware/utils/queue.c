#include <stdlib.h>
#include "queue.h"

void enqueue(event_t new_event){
    node* current_node = (node*)malloc(sizeof(node));

    if(current_node == NULL){
        printf("Couldnt create node\r\n");
        return;
    }
    current_node -> event = new_event;
    current_node -> next = NULL;

    if(event_queue.tail == NULL){
        event_queue.head = event_queue.tail = current_node;
    }else{
        event_queue.tail->next = current_node;
        event_queue.tail = current_node;
    }
}

void print_queue(){
    if(is_queue_empty() == true) return;
    node* current_node = event_queue.head;

    uint8_t i = 0;
    while(current_node!=NULL){
        printf("Event queued in %d position: %d\r\n", i, current_node->event);
        i ++;
        current_node = current_node->next;
    }
    printf("\r\n");
}

event_t dequeue(){
    if(is_queue_empty() == true) return -1;

    node* current_node = event_queue.head;
    event_t event_buffer = current_node->event;
    event_queue.head = event_queue.head->next;

    if(event_queue.head == NULL) event_queue.tail = NULL;
    
    free(current_node); 
    return event_buffer;
}

bool is_queue_empty(){
    if(event_queue.head == NULL){
        printf("Empty list\r\n");
        return true;
    }
    return false;
}