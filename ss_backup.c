#include "headers.h"

void back_make_file(char* file, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    int ack = mkdir(file, 0777);
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        return;
    }
    if(ack < 0)
    {
        perror("[-]Directory create error");
        return;
    }
}

void folderBanao(char* buffer_nm, int nm_sockfd)
{
    char* token = strtok(buffer_nm, " ");
    token = strtok(NULL, " ");

    if(mkdir(token, 0777) < 0)
    {
        if(send(nm_sockfd, "-1", strlen("-1"), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]Directory create error");
        exit(1);
    }
    if(send(nm_sockfd, "ack", strlen("ack"), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    recvBackup(nm_sockfd);
}

void fileChepo(char* buffer_nm_2, int nm_sockfd)
{
    if(send(nm_sockfd, "ack", strlen("ack"), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    char buffer_nm[1024];
    char* token = strtok(buffer_nm_2, " ");
    token = strtok(NULL, " ");
    FILE* fd = fopen(token, "w+");
    // printf("[+]Creating file %s\n", token);
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        exit(1);
    }
    // printf("[+]Receiving %s\n", buffer_nm);
    while((strncmp(buffer_nm, "create_file", strlen("create_file")) != 0) && strncmp(buffer_nm, "create_folder", strlen("create_folder")) != 0 && (strcmp(buffer_nm, "__DONE__") != 0))
    {
        fprintf(fd, "%s", buffer_nm);
        bzero(buffer_nm, 1024);
        if(send(nm_sockfd, "ack", strlen("ack"), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        // printf("Hmmm\n");
        if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Recv error");
            exit(1);
        }
        // printf("[+]Receiving %s\n", buffer_nm);
    }
    fclose(fd);
    // printf("Finally\n");
    if(strcmp(buffer_nm, "__DONE__") == 0)
    {
        bzero(buffer_nm, 1024);
        strcpy(buffer_nm, "1");
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        return;
    }
    else if(strncmp(buffer_nm, "create_folder", strlen("create_folder")) == 0)
        folderBanao(buffer_nm, nm_sockfd);
    else if(strncmp(buffer_nm, "create_file", strlen("create_file")) == 0)
    {
        fileChepo(buffer_nm, nm_sockfd);
    }
}

void recvBackup(int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        exit(1);
    }
    if(strcmp(buffer_nm, "__DONE__") == 0)
    {
        bzero(buffer_nm, 1024);
        strcpy(buffer_nm, "1");
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        return;
    }
    else if(strncmp(buffer_nm, "create_file", strlen("create_file")) == 0)
        fileChepo(buffer_nm, nm_sockfd);
    else if(strncmp(buffer_nm, "create_folder", strlen("create_folder")) == 0)
        folderBanao(buffer_nm, nm_sockfd);
}