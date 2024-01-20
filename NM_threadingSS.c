#include "headers.h"

#define NUM_STORAGE_SERVERS 100
#define naming_server_port 4545
char *nm_ip = "127.0.0.1";

extern trie *root;
extern lru_head *head;
extern ss_info *array_of_ss_info;

void *Handle_SS()
{
    int server_sock, ss_as_client_sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in ss_as_client_addr;
    socklen_t addr_size;
    char buffer[1024];

    // Create socket for the connection
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("[-]Socket error");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(naming_server_port);
    server_addr.sin_addr.s_addr = inet_addr(nm_ip);

    // Bind the server socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("[-]Bind error");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_sock, NUM_STORAGE_SERVERS) == -1)
    {
        perror("[-]Listen error");
        exit(1);
    }
    printf("[+]Listening for Storage Servers...\n");

    int count = 0;

    while (count < NUM_STORAGE_SERVERS)
    {
        addr_size = sizeof(ss_as_client_addr);
        ss_as_client_sock = accept(server_sock, (struct sockaddr *)&ss_as_client_addr, &addr_size);

        if (ss_as_client_sock < 0)
        {
            perror("[-]Accept error");
            exit(1);
        }

        // Receive initialization details from Storage Server
        bzero(buffer, sizeof(buffer));
        if (recv(ss_as_client_sock, buffer, sizeof(buffer), 0) < 0)
        {
            perror("[-]Receive error");
            exit(1);
        }

        // Extract IP, Naming Server Port, and Client Port from the received buffer
        char ss_ip[21];
        int ss_num, ss_client_port, ss_nm_port;
        sscanf(buffer, "SS_NUM: %d\nSS_CLIENT_PORT: %d\nSS_NM_PORT: %d\nSS_IP: %s\n", &ss_num, &ss_client_port, &ss_nm_port, ss_ip);

        // Insert Storage Server information into the array
        insert_ss_info(ss_num, ss_client_port, ss_nm_port, ss_ip);

        // Extract file paths and insert them into Tries
        while (1)
        {
            bzero(buffer, sizeof(buffer));
            if (recv(ss_as_client_sock, buffer, sizeof(buffer), 0) < 0)
            {
                perror("[-]Receive error");
                exit(1);
            }

            if (strcmp(buffer, "DONE") == 0)
            {
                // All paths received
                break;
            }

            // Insert into Trie
            insert(root, buffer, count);
        }
        count++;

        // Acknowledge connection
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "Connection successful");

        if (send(ss_as_client_sock, buffer, sizeof(buffer), 0) < 0)
        {
            perror("[-]Send error");
            exit(1);
        }

        close(ss_as_client_sock);
    }

    // Print Storage Server information
    printf("[+]Storage Server Information:\n");
    print_ss_info();

    // Close the server socket
    close(server_sock);

    pthread_exit(NULL);
}

int main()
{
    // Initialize Tries
    trie *root = init();

    // Initialize Storage Server information
    init_ss_info();

    // Creating a thread to handle Storage Server requests and sending trie as argument
    pthread_t ss_thread;
    pthread_create(&ss_thread, NULL, Handle_SS, NULL);

    // Naming Server code: handling client requests (Abhinav)

    pthread_join(ss_thread, NULL);

    // Print all paths stored in the Trie
    printf("[+]Paths stored in Trie:\n");
    print_all_strings_in_trie(root);

    return 0;
}