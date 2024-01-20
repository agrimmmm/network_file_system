#ifndef __SS_BACKUP_H__
#define __SS_BACKUP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

void back_make_file(char* file, int nm_sockfd);
void recvBackup(int nm_sockfd);

#endif