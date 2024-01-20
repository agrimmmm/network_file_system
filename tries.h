#ifndef TRIES_H
#define TRIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readwritelock.h"

//tries in c for all ascii characters

typedef struct node
{
    int is_end;     //also the same as server_num
    struct node *next[128];
    rwlock_t rwlock;
}trie;

#define MAX_STRINGS 1024
#define MAX_STRING_LENGTH 1024

trie* init();
void insert(trie* root, char* str, int server_num);
int search(trie* root, char* str);
rwlock_t* find_rwlock(trie* root, char* str);
void print_trie(trie* root, char* prefix);
void print_all_strings_in_trie(trie* root);
void delete_node(trie* root, char* str);
void collect_strings_with_prefix(trie *node, char *current_prefix);
void search_and_insert(trie *root, char *prefix);
void print_result_strings();

#endif