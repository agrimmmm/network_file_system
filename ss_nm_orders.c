#include "ss_nm_orders.h"

extern ss_trie* ss_root;

void make_file(char* file, int nm_sockfd)
{
    // printf("create file %s\n", file);
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    // int fd = open(file, O_CREAT | O_RDWR, 0777);
    // printf("%s\n", file);
    FILE* fd = fopen(file, "w+");
    if(fd != NULL)
    {
        ss_insert(ss_root, file);
        bzero(buffer_nm, 1024);
        strcpy(buffer_nm, "1");
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
    }
    else
    {
        bzero(buffer_nm, 1024);
        strcpy(buffer_nm, "-1");
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]File create error");
        //error handling to be done
        //dont exit program but just return
        // exit(1);
        return;
    }
    // sprintf(buffer_nm, "%d", fd);
    // if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    // {
    //     perror("[-]Send error");
    //     exit(1);
    // }
    // if(fd < 0)
    // {
    //     perror("[-]File create error");
    //     //error handling to be done
    //     //dont exit program but just return
    //     exit(1);
    // }
    fclose(fd);
}

void del_file(char* file, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    int ack = remove(file);
    sprintf(buffer_nm, "%d", ack);
    if(ack == 0)
    {
        ss_delete_node(ss_root, file);
        if(send(nm_sockfd, "1", strlen("1"), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
    }
    else
    {
        if(send(nm_sockfd, "-1", strlen("-1"), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]File delete error");
        exit(1);
    }
}

void delete_dir(char* name, int nm_sockfd)
{
    int ack = 0;
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    if(ack == -1)
    {
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            return;
        }
        perror("[-]Directory delete error");
        return;
    }
    DIR* dir = opendir(name);
    if(dir == NULL)
    {
        perror("[-]Directory delete error");
        exit(1);
    }
    struct dirent* entry = readdir(dir);
    struct stat statbuff;
    while(entry != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            entry = readdir(dir);
            continue;
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if(stat(path, &statbuff) < 0)
        {
            perror("[-]File stat error");
            return;
        }
        if(S_ISDIR(statbuff.st_mode))
            delete_dir(path, nm_sockfd);
        else
        {
            ack = remove(path);
            if(ack < 0)
            {
                perror("[-]File delete error");
                return;
            }
            ss_delete_node(ss_root, path);
        }
        entry = readdir(dir);
    }
    rmdir(name);
    ss_delete_node(ss_root, name);
}

void make_dir(char* name, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    int ack = mkdir(name, 0777);
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
    ss_insert(ss_root, name);
}

void copyFile(char* file, char* dir, int nm_sockfd)
{
    printf("File: %s\tDir: %s\n", file, dir);
    int ack = -1;
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);

    char* temp = (char*)malloc(sizeof(char) * (strlen(file) + 1));
    strcpy(temp, file);
    char file_name[1024];
    char* token = strtok(temp, "/");
    while(token != NULL)
    {
        bzero(file_name, 1024);
        strcpy(file_name, token);
        token = strtok(NULL, "/");
    }

    char* new_file = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(file) + 5));
    sprintf(new_file, "%s/%s", dir, file_name);
    if(strcmp(file, new_file) == 0)
    {
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            return;
        }
        perror("[-]File copy error");
        return;
    }

    FILE* fd1 = fopen(file, "r");
    FILE* fd2 = fopen(new_file, "w+");
    if(fd1 == NULL || fd2 == NULL)
    {
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]File open error");
        return;
    }
    ss_insert(ss_root, new_file);
    char ch;
    while((ch = fgetc(fd1)) != EOF)
        fputc(ch, fd2);
    fclose(fd1);
    fclose(fd2);
    ack = 1;
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
}

void recvFileFromSS(char* file, char* dest, int nm_sockfd)
{
    int ack = -1;
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        return;
    }
    char new_file[1024];
    // printf("SS2 %s\n", buffer_nm);
    if(strncmp(buffer_nm, "create_file", strlen("create_file")) == 0)
    {
        char* token = strtok(buffer_nm, " ");
        token = strtok(NULL, " ");
        make_file(token, nm_sockfd);
        strcpy(new_file, token);
    }

    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        return;
    }
    int num_packets = atoi(buffer_nm);
    bzero(buffer_nm, 1024);
    if(send(nm_sockfd, "ack", strlen("ack"), 0) < 0)
    {
        perror("[-]Send error");
        return;
    }
    printf("%d\n", num_packets);
    FILE* fd = fopen(new_file, "a");
    while(num_packets-- > 0)
    {
        // printf("num_packets: %d\n", num_packets);
        bzero(buffer_nm, 1024);
        if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Recv error");
            return;
        }
        // printf("%s\n", buffer_nm);
        fprintf(fd, "%s", buffer_nm);
    }
    fclose(fd);
    // printf("exited\n");
    // fprintf(fd, "\n");
    ack = 1;
    bzero(buffer_nm, 1024);
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        return;
    }
}

void sendFileToSS(char* file, char* dest, int nm_sockfd)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);

    char file_name[1024];
    char* temp = (char*)malloc(sizeof(char) * (strlen(file) + 1));
    strcpy(temp, file);
    char* token = strtok(temp, "/");
    while(token != NULL)
    {
        bzero(file_name, 1024);
        strcpy(file_name, token);
        token = strtok(NULL, "/");
    }

    char* new_file = (char*)malloc(sizeof(char) * (strlen(dest) + strlen(file) + 5));
    bzero(new_file, strlen(dest) + strlen(file) + 5);
    sprintf(new_file, "%s/%s", dest, file_name);
    sprintf(buffer_nm, "create_file %s", new_file);
    // printf("SS1 %s\n", buffer_nm);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        return;
    }
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        return;
    }

    read_file(file, nm_sockfd);
}