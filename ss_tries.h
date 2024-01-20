#ifndef __S_TRIES_H
#define __S_TRIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tries in c for all ascii characters

typedef struct ss_node
{
    int is_end;     //also the same as server_num
    struct ss_node *next[128];
}ss_trie;

ss_trie* ss_init();
void ss_insert(ss_trie* root, char* str);
int ss_search(ss_trie* root, char* str);
void ss_print_trie(ss_trie* root, char* prefix);
void ss_print_all_strings_in_trie(ss_trie* root);
void ss_delete_node(ss_trie* root, char* str);

#endif