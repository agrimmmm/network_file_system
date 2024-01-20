#ifndef __NM_H__
#define __NM_H_

#include "headers.h"

int read_or_retrieve_file(char *input, int nm_sock_for_client);
void* main_backup_file(void* args);
void do_backup_file(int ss_num, char* input);
void backup_create_file(int ss_num,int backup_num,char* input);

#endif