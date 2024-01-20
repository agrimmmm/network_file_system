#include "headers.h"
#include "ss_tries.h"

#define PORT 4545
char* ip = "127.0.0.1";

char* Read = "read";
char* Write = "write";
char* Retrieve = "retrieve";
char* create_file = "create_file";
char* delete_file = "delete_file";
char* copy_file = "copy_file";
char* create_folder = "create_folder";
char* delete_folder = "delete_folder";
char* copy_folder = "copy_folder";
char* back_folder = "backup_folder";
char* back_create = "backcreate_folder";

typedef struct filepaths* files;
typedef struct filepaths
{
    char* name;
    files next;
}filepaths;

ss_trie* ss_root;

typedef struct threadargs* args;
typedef struct threadargs
{
    socklen_t addr_size;
    struct sockaddr_in server_addr;
    int server_sock;
    int sockfd;
    int port;
    // int ss_port;
}threadargs;

typedef struct clientargs* clargs;
typedef struct clientargs
{
    socklen_t addr_size;
    struct sockaddr_in server_addr;
    int server_sock;
    int sockfd;
}clientargs;

void remove_nextline(char* str)
{
    while(str[strlen(str)-1] == '\n')
    {
        str[strlen(str)-1] = '\0';
    }
}

void* nm_handler(int nm_sockfd, int port, int client_port, char* ip, char* paths_of_all)
{
    char buffer_nm[1024];
    bzero(buffer_nm, 1024);
    sprintf(buffer_nm, "IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\n", ip, port, client_port);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Receive error");
        exit(1);
    }
    bzero(buffer_nm, 1024);
    strcpy(buffer_nm, paths_of_all);
    if(send(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Send error");
        exit(1);
    }
    bzero(buffer_nm, 1024);
    if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
    {
        perror("[-]Receive error");
        exit(1);
    }

    close(nm_sockfd);
}

void* nm_handler_for_ops(void* arg)
{
    args args_nm = (args)arg;
    socklen_t addr_size = args_nm->addr_size;
    struct sockaddr_in server_addr_1 = args_nm->server_addr;
    int server_sock_1 = args_nm->server_sock;
    int nm_sockfd = args_nm->sockfd;
    char buffer_nm[1024];
    int nm_port = args_nm->port;

    server_sock_1 = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock_1 < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    memset(&server_addr_1, '\0', sizeof(server_addr_1));
    server_addr_1.sin_family = AF_INET;
    server_addr_1.sin_port = htons(nm_port);
    server_addr_1.sin_addr.s_addr = inet_addr(ip);
    if(bind(server_sock_1, (struct sockaddr*)&server_addr_1, sizeof(server_addr_1)) < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to port for naming server\n");
    int l = listen(server_sock_1, 100);
    if(l < 0)
    {
        perror("[-]Listen error NM");
        exit(1);
    }


    while(1)
    {
        addr_size = sizeof(server_addr_1);
        nm_sockfd = accept(server_sock_1, (struct sockaddr*)&server_addr_1, &addr_size);
        if(nm_sockfd < 0)
        {
            perror("[-]Accept error NM");
            exit(1);
        }
        printf("[+]Naming server connected.\n");

        bzero(buffer_nm, 1024);
        if(recv(nm_sockfd, buffer_nm, sizeof(buffer_nm), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        if(strncmp(create_file, buffer_nm, strlen(create_file)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);

            make_file(file, nm_sockfd);
        }
        else if(strncmp(delete_file, buffer_nm, strlen(delete_file)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);

            del_file(file, nm_sockfd);
        }
        else if(strncmp(delete_folder, buffer_nm, strlen(delete_folder)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);

            delete_dir(file, nm_sockfd);
        }
        else if(strncmp(create_folder, buffer_nm, strlen(create_folder)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);

            make_dir(file, nm_sockfd);
        }
        else if(strncmp(copy_file, buffer_nm, strlen(copy_file)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* source = token;
            remove_nextline(source);
            token = strtok(NULL, " ");
            char* dest = token;
            remove_nextline(dest);
            char* port = strtok(NULL, " ");
            remove_nextline(port);
            if(strcmp(port, "same") == 0)
                copyFile(source, dest, nm_sockfd);
            else if(strcmp(port, "receive") == 0)
                recvFileFromSS(source, dest, nm_sockfd);
            else if(strcmp(port, "send") == 0)
                sendFileToSS(source, dest, nm_sockfd);
        }
        else if(strncmp(copy_folder, buffer_nm, strlen(copy_folder)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* source = token;
            remove_nextline(source);
            token = strtok(NULL, " ");
            char* dest = token;
            remove_nextline(dest);
            token = strtok(NULL, " ");
            char* comm = token;
            if(strcmp(comm, "same") == 0)
                aurNahiHota(source, dest, nm_sockfd);
            else if(strcmp(comm, "receive") == 0)
                recvDirFromSS(nm_sockfd);
            else if(strcmp(comm, "send") == 0)
                recursivelySend(source, dest, nm_sockfd);
        }
        else if(strncmp(back_folder, buffer_nm, strlen(back_folder)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* source = token;
            remove_nextline(source);
            token = strtok(NULL, " ");
            char* dest = token;
            remove_nextline(dest);
            token = strtok(NULL, " ");
            char* comm = token;
            if(strcmp(comm, "receive") == 0)
                recvBackup(nm_sockfd);
            else if(strcmp(comm, "send") == 0)
                recursivelySend(source, dest, nm_sockfd);
        }
        else if(strncmp(back_create, buffer_nm, strlen(back_create)) == 0)
        {
            char* token = strtok(buffer_nm, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);

            back_make_file(file, nm_sockfd);
        }
        close(nm_sockfd);
    }
    close(server_sock_1);
    pthread_exit(NULL);
}

void* client_handler_for_ops(void* arg)
{
    clargs args = (clargs)arg;
    socklen_t addr_size = args->addr_size;
    struct sockaddr_in server_addr_2 = args->server_addr;
    int server_sock_2 = args->server_sock;
    int client_sockfd = args->sockfd;
    char buffer_client[1024];
    
    while(1)
    {
        addr_size = sizeof(server_addr_2);
        client_sockfd = accept(server_sock_2, (struct sockaddr*)&server_addr_2, &addr_size);
        if(client_sockfd < 0)
        {
            perror("[-]Accept error Client");
            exit(1);
        }
        printf("[+]Client connected.\n");

        //input from client
        bzero(buffer_client, 1024);
        if(recv(client_sockfd, buffer_client, sizeof(buffer_client), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }
        // printf("input from client: %s\n", buffer_client);
        if(strncmp(buffer_client, Read, strlen(Read)) == 0)
        {
            char* token = strtok(buffer_client, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);

            read_file(file, client_sockfd);
        }
        else if(strncmp(buffer_client, Write, strlen(Write)) == 0)
        {
            //send ready to receive data for writing
            if(send(client_sockfd, "OK", sizeof("OK"), 0) < 0)
            {
                perror("[-]Send error");
                exit(1);
            }
            char* token = strtok(buffer_client, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);
            printf("file %s\n", file);
            write_file(file, client_sockfd);
        }
        else if(strncmp(buffer_client, Retrieve, strlen(Retrieve)) == 0)
        {
            char* token = strtok(buffer_client, " ");
            token = strtok(NULL, " ");
            char* file = token;
            remove_nextline(file);
            
            retrieve_info(file, client_sockfd);
        }
        close(client_sockfd);
    }
}

void* client_handler(void* arg)
{
    args args_client = (args)arg;
    socklen_t addr_size = args_client->addr_size;
    struct sockaddr_in server_addr_2 = args_client->server_addr;
    int server_sock_2 = args_client->server_sock;
    int client_sockfd = args_client->sockfd;
    char buffer_client[1024];
    int client_port = args_client->port;

    server_sock_2 = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock_2 < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    memset(&server_addr_2, '\0', sizeof(server_addr_2));
    server_addr_2.sin_family = AF_INET;
    server_addr_2.sin_port = htons(client_port);
    server_addr_2.sin_addr.s_addr = inet_addr(ip);
    if(bind(server_sock_2, (struct sockaddr*)&server_addr_2, sizeof(server_addr_2)) < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    printf("[+]Bind to port for clients\n");
    int l = listen(server_sock_2, 100);
    if(l < 0)
    {
        perror("[-]Listen error Client");
        exit(1);
    }

    pthread_t client_thread[100];
    clargs* thclargs = (clargs*)calloc(sizeof(clargs), 100);
    for(int i=0; i<100; i++)
    {
        thclargs[i] = (clargs)malloc(sizeof(clientargs));
        thclargs[i]->addr_size = addr_size;
        thclargs[i]->server_addr = server_addr_2;
        thclargs[i]->server_sock = server_sock_2;
        thclargs[i]->sockfd = client_sockfd;
        pthread_create(&client_thread[i], NULL, client_handler_for_ops, (void*)thclargs[i]);
    }

    for(int i=0; i<100; i++)
    {
        pthread_join(client_thread[i], NULL);
    }
    
    close(server_sock_2);
    pthread_exit(NULL);
}


//storage server implementation for NFS
int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage: %s <nm_port> <client_port>\n", argv[0]);
        exit(1);
    }
    
    int nm_port = atoi(argv[1]);
    int client_port = atoi(argv[2]);
    // int ss_port = atoi(argv[3]);
    // int nm_port = 4040;
    // int client_port = 7070;

    ss_root = ss_init();

    printf("IP: %s\nNM_PORT: %d\nCLIENT_PORT: %d\nEnter file paths(relative to current directory) that the clients can access: (Enter 'DONE' when finished)\n", ip, nm_port, client_port);

    files head = (files)malloc(sizeof(filepaths));
    files temp = head;
    while(1)
    {
        char* path = (char*)calloc(sizeof(char), 100);
        scanf("%s", path);
        if(strcmp(path, "DONE") == 0)
            break;
        ss_insert(ss_root, path);
        temp->name = (char*)malloc(sizeof(char)*100);
        strcpy(temp->name, path);
        temp->next = (files)malloc(sizeof(filepaths));
        temp = temp->next;
    }

    //concatenation of file names
    char* paths_of_all = (char*)malloc(sizeof(char)*60000);
    strcpy(paths_of_all, head->name);
    head = head->next;
    while(head->name != NULL)
    {
        strcat(paths_of_all, "|");
        strcat(paths_of_all, head->name);
        head = head->next;
    }
    strcat(paths_of_all, "|");
    // printf("%s\n", paths_of_all);
    
    //defining sockets and buffers
    int socky, server_sock_1, server_sock_2, nm_sockfd, client_sockfd;
    struct sockaddr_in server_addr_1, server_addr_2, nm_addr, cliett_addr;
    socklen_t addr_size;

    socky = socket(PF_INET, SOCK_STREAM, 0);
    if(server_sock_1 < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }

    //connecting to naming server as a client
    memset(&server_addr_1, '\0', sizeof(server_addr_1));
    server_addr_1.sin_family = AF_INET;
    server_addr_1.sin_port = htons(PORT);
    server_addr_1.sin_addr.s_addr = inet_addr(ip);

    if(connect(socky, (struct sockaddr*)&server_addr_1, sizeof(server_addr_1)) == -1)
    {
        perror("[-]Connect error");
        exit(1);
    }
    //sending storage server info to naming server
    nm_handler(socky, nm_port, client_port, ip, paths_of_all);
    printf("[+]Naming server info sent.\n");

    pthread_t input[2];
    addr_size = sizeof(server_addr_1);

    //thread for naming server
    args args_nm = (args)malloc(sizeof(threadargs));
    args_nm->addr_size = addr_size;
    args_nm->server_addr = server_addr_1;
    args_nm->server_sock = server_sock_1;
    args_nm->sockfd = nm_sockfd;
    args_nm->port = nm_port;
    if(pthread_create(&input[0], NULL, nm_handler_for_ops, (void*)args_nm) != 0)
        printf("[-]Thread creation error.\n");
    sleep(0.001);

    //thread for client
    addr_size = sizeof(server_addr_2);
    args args_client = (args)malloc(sizeof(threadargs));
    args_client->addr_size = addr_size;  
    args_client->server_addr = server_addr_2;
    args_client->server_sock = server_sock_2;
    args_client->sockfd = client_sockfd;
    args_client->port = client_port;
    if(pthread_create(&input[1], NULL, client_handler, (void*)args_client) != 0)
        printf("[-]Thread creation error.\n");
    sleep(0.001);

    for(int i=0; i<2; i++)
    {
        pthread_join(input[i], NULL);
    }

    return 0;
}