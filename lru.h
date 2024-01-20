#ifndef __LRU_H__
#define __LRU_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LRU_CACHE_SIZE 10

typedef struct lru_head
{
    int num_nodes;
    struct lru_node* front;
    struct lru_node* rear;
}lru_head;

//front is latest and rear is oldest

typedef struct lru_node
{
    char* filepath;
    int storage_server_num;
    int storage_server_port_for_client;
    char* storage_server_ip;
    struct lru_node* next;
    struct lru_node* prev;
}lru_node;

lru_node* find_and_return(char* filepath, lru_head* head);
lru_head* init_lru();
lru_node* make_lru_node(char* filepath, int storage_server_num, int storage_server_port_for_client, char* storage_server_ip);
void insert_at_front(lru_node* node, lru_head* head);
lru_node* delete_lru_node(char* filepath, lru_head* head);
void shift_node_to_front(char* filepath, lru_head* head);
void print_lru(lru_head* head);

#endif