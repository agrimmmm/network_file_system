#include "headers.h"

extern ss_trie* ss_root;

void aurNahiHota(char* dir, char* dest, int nm_sockfd)
{
    copyDir(dir, dest, nm_sockfd);
    int ack = 1;
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    sprintf(buffer_nm, "%d", ack);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        return;
    }
}

void copyDir(char* dir, char* dest, int nm_sockfd)
{
    printf("dir: %s\tdest: %s\n", dir, dest);
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    
    DIR* dirp = opendir(dir);
    if(dirp == NULL)
    {
        int ack = -1;
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            return;
        }
        perror("[-]Directory open error");
        exit(1);
    }

    char dir_name[100];
    char temp[1024];
    strcpy(temp, dir);
    char* token = strtok(temp, "/");
    while(token != NULL)
    {
        bzero(dir_name, 100);
        strcpy(dir_name, token);
        token = strtok(NULL, "/");
    }

    char* new_dir = (char*)malloc(sizeof(char) * (strlen(dest) + strlen(dir) + 5));
    bzero(new_dir, sizeof(new_dir));
    sprintf(new_dir, "%s/%s", dest, dir_name);
    if(strcmp(dir, new_dir) == 0)
    {
        int ack = -1;
        sprintf(buffer_nm, "%d", ack);
        if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]Folder copy error");
        return;
    }
    mkdir(new_dir, 0777);
    ss_insert(ss_root, new_dir);

    struct dirent* entry = readdir(dirp);
    struct stat statbuff;
    while(entry != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            entry = readdir(dirp);
            continue;
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        printf("Path: %s\n", path);
        if(ss_search(ss_root, path) <= 0)
        {
            entry = readdir(dirp);
            continue;
        }
        printf("Found in trie\n");
        if(stat(path, &statbuff) < 0)
        {
            perror("[-]File stat error");
            continue;
        }
        if(S_ISDIR(statbuff.st_mode))
            copyDir(path, new_dir, nm_sockfd);
        else
            copyFile(path, new_dir, nm_sockfd);
        entry = readdir(dirp);
    }
}

void makeFolder(char* buffer_nm, int nm_sockfd)
{
    // printf("HH %s\n", buffer_nm);
    char* token = strtok(buffer_nm, " ");
    token = strtok(NULL, " ");
    // printf("[+]Creating folder %s\n", token);
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
    ss_insert(ss_root, token);
    if(send(nm_sockfd, "ack", strlen("ack"), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    recvDirFromSS(nm_sockfd);
}

void fileBanao(char* buffer_nm_2, int nm_sockfd)
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
    ss_insert(ss_root, token);
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
        makeFolder(buffer_nm, nm_sockfd);
    else if(strncmp(buffer_nm, "create_file", strlen("create_file")) == 0)
    {
        fileBanao(buffer_nm, nm_sockfd);
    }
}

void recvDirFromSS(int nm_sockfd)
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
    {
        fileBanao(buffer_nm, nm_sockfd);
    }
    else if(strncmp(buffer_nm, "create_folder", strlen("create_folder")) == 0)
        makeFolder(buffer_nm, nm_sockfd);
    // printf("Ruk ja bas\n");
}

void filesender(char* file, char* dir, int nm_sockfd)
{
    char buffer[1024];
    bzero(buffer, 1024);

    char file_name[100];
    char temp[1024];
    strcpy(temp, file);
    char* token = strtok(temp, "/");
    while(token != NULL)
    {
        bzero(file_name, 100);
        strcpy(file_name, token);
        token = strtok(NULL, "/");
    }

    sprintf(buffer, "create_file %s/%s", dir, file_name);
    if(send(nm_sockfd, buffer, sizeof(buffer), 0) < 0)
    {
        perror("[-]Send error");
        exit;
    }
    bzero(buffer, 1024);
    if(recv(nm_sockfd, buffer, sizeof(buffer), 0) < 0)
    {
        perror("[-]Recv error");
        exit;
    }

    FILE* fd = fopen(file, "r");
    bzero(buffer, 1024);
    while(fgets(buffer, sizeof(buffer), fd) != NULL)
    {
        if(send(nm_sockfd, buffer, sizeof(buffer), 0) < 0)
        {
            perror("[-]Send error");
            exit;
        }
        bzero(buffer, 1024);
        if(recv(nm_sockfd, buffer, sizeof(buffer), 0) < 0)
        {
            perror("[-]Recv error");
            exit;
        }
        bzero(buffer, 1024);
    }
}

void sendDirToSS(char* dir, char* dest, int nm_sockfd)
{
    // printf("HI\n");
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);

    DIR* dirp = opendir(dir);
    if(dirp == NULL)
    {
        if(send(nm_sockfd, "-1", strlen("-1"), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }
        perror("[-]Directory open error");
        exit(1);
    }
    // printf("BYE\n");

    char dir_name[100];
    char temp[1024];
    strcpy(temp, dir);
    char* token = strtok(temp, "/");
    while(token != NULL)
    {
        bzero(dir_name, 100);
        strcpy(dir_name, token);
        token = strtok(NULL, "/");
    }
    // printf("dir_name: %s\n", dir_name);

    char* new_dir = (char*)malloc(sizeof(char) * (strlen(dest) + strlen(dir) + 5));
    bzero(new_dir, sizeof(new_dir));
    // printf("destdir: %s\n", dest);
    sprintf(new_dir, "%s/%s", dest, dir_name);
    sprintf(buffer_nm, "create_folder %s", new_dir);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        exit(1);
    }

    struct dirent* entry = readdir(dirp);
    struct stat statbuff;
    while(entry != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            entry = readdir(dirp);
            continue;
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if(ss_search(ss_root, path) <= 0)
        {
            entry = readdir(dirp);
            continue;
        }
        if(stat(path, &statbuff) < 0)
        {
            perror("[-]File stat error");
            continue;
        }
        if(S_ISDIR(statbuff.st_mode))
            sendDirToSS(path, new_dir, nm_sockfd);
        else
            filesender(path, new_dir, nm_sockfd);
        entry = readdir(dirp);
    }
}

void recursivelySend(char* dir, char* dest, int nm_sockfd)
{
    sendDirToSS(dir, dest, nm_sockfd);
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    strcpy(buffer_nm, "__DONE__");
    // printf("END1\n");
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    // printf("END2\n");
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Recv error");
        exit(1);
    }
    // printf("END3\n");
}