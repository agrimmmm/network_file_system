#ifndef __SS_COPYDIR_H__
#define __SS_COPYDIR_H__

void aurNahiHota(char* dir, char* dest, int nm_sockfd);
void copyDir(char* dir, char* dest, int nm_sockfd);
void makeFolder(char* buffer_nm, int nm_sockfd);
void fileBanao(char* buffer_nm, int nm_sockfd);
void recvDirFromSS(int nm_sockfd);
void filesender(char* file, char* dir, int nm_sockfd);
void sendDirToSS(char* dir, char* dest, int nm_sockfd);
void recursivelySend(char* dir, char* dest, int nm_sockfd);

#endif