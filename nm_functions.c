#include "headers.h"
#include "nm.h"

extern trie *root;
extern lru_head *head;
extern ss_info *array_of_ss_info;
char result_strings[MAX_STRINGS][MAX_STRING_LENGTH];
int result_count;
// receive action from client
// for read: read filename
// for write: write filename and then after receiving ack write data
// for retrieve information: retrieve filename
// for create: create file filepath
// for delete: delete filename with path
// for copy file: copy filepath to newfilepath
// for create folder: create folder folderpath
// for delete folder: delete folder folderpath
// for copy folder: copy folder folderpath to newfolderpath

int what_to_do(char *input, int nm_sock_for_client)
{
    printf("task of nm server: %s\n", input);
    if ((strncmp(input, "read", strlen("read")) == 0) || (strncmp(input, "retrieve", strlen("retrieve")) == 0))
    {
        read_or_retrieve_file(input, nm_sock_for_client);
        return 1;
    }
    else if (strncmp(input, "write", strlen("write")) == 0)
    {
        // command: write filepath
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        // Check if the file is in the LRU cache
        lru_node *node_in_cache = find_and_return(file_path, head);
        // File not found in cache
        if (node_in_cache == NULL)
        {
            // printf("not in cache\n");
            // Search in trie
            int ss_num = search(root, file_path);
            // printf("ss_num: %d\n", ss_num);

            if (ss_num == 0)
            {
                // If file not found in trie, send -1 as acknowledgment to the client
                printf("file not found!!!\n");
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }
            else
            {
                // printf("found in trie\n");
                // File found in trie, send storage server details to the client
                char send_details_to_client[BUF_SIZE];
                int port = array_of_ss_info[ss_num].ss_client_port;
                strcpy(send_details_to_client, array_of_ss_info[ss_num].ss_ip);

                // Concatenate IP and port as a string separated by space
                char port_as_string[100];
                sprintf(port_as_string, " %d", port);
                strcat(send_details_to_client, port_as_string);

                rwlock_t *rwlock_t = find_rwlock(root, file_path);
                acquire_writelock(rwlock_t);
                printf("write lock acquired\n");
                printf("ss details sent to the client: %s#\n", send_details_to_client);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                // receive ack from client
                char ack[BUF_SIZE];
                if (recv(nm_sock_for_client, ack, sizeof(ack), 0) < 0)
                {
                    perror("recv() error");
                    exit(1);
                }
                release_writelock(rwlock_t);
                printf("write lock released\n");
            }
        }
        // File found in the LRU cache
        else
        {
            // Send storage server details to the client
            char send_details_to_client[BUF_SIZE];
            int port = node_in_cache->storage_server_port_for_client;
            char ss_ip[21];
            strcpy(ss_ip, node_in_cache->storage_server_ip);

            // Concatenate IP and port as a string separated by space
            sprintf(send_details_to_client, "%s %d", ss_ip, port);
            printf("ss details sent to the client: %s#\n", send_details_to_client);
            rwlock_t *rwlock_t = find_rwlock(root, file_path);
            acquire_writelock(rwlock_t);
            printf("write lock acquired\n");
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            // receive ack from client
            char ack[BUF_SIZE];
            if (recv(nm_sock_for_client, ack, sizeof(ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }
            release_writelock(rwlock_t);
            printf("write lock released\n");
        }
        return 1;
    }

    else if (strncmp(input, "create_file", strlen("create_file")) == 0)
    {
        // if file already exists send ack as -1
        // choose ss number by comparing string by removing file name
        // make connection with storage server and send action
        // receive ack from storage server
        // add to trie the file and storage server number
        // add to lru cache

        // command = create_file filepath
        // Get the file path from the command
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        // Checking if the file already exists in the trie
        if (search(root, file_path) > 0)
        {
            // Send -1 acknowledgment to the client
            printf("file already exists!!!\n");
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", -1);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            return 1;
        }
        else
        {
            // Extract directory path
            char dir_path[1024];
            strcpy(dir_path, file_path);

            char *remove_file_name = strrchr(dir_path, '/');
            if (remove_file_name != NULL)
                *remove_file_name = '\0';

            // Find Storage Server number
            int ss_num = search(root, dir_path);

            // Handle directory not found
            if (ss_num == 0)
            {
                printf("directory not found!!!\n");
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }

            // Retrieve Storage Server information
            int ss_client_port = array_of_ss_info[ss_num].ss_client_port;
            int ss_nm_port = array_of_ss_info[ss_num].ss_nm_port;
            char *ss_ip = array_of_ss_info[ss_num].ss_ip;

            // Make a connection with the Storage Server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buffer[1024];

            // Create socket
            sock2 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr2, 0, sizeof(serv_addr2));
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = inet_addr(ss_ip);
            serv_addr2.sin_port = htons(ss_nm_port);

            // Connect to Storage Server
            if (connect(sock2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            printf("Connected to Storage Server %d\n", ss_num);

            // Send input to Storage Server
            if (send(sock2, input, strlen(input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive Ack from Storage Server
            int ack;
            char buffer_ack[BUF_SIZE];
            if (recv(sock2, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Convert received acknowledgment to an integer
            sscanf(buffer_ack, "%d", &ack);

            close(sock2);

            // Send the same acknowledgment to the client
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", ack);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            if (ack >= 0)
            {
                // Insert into trie
                insert(root, file_path, ss_num);

                // Insert into LRU
                // lru_node *new_lru_node = make_lru_node(file_path, ss_num, ss_client_port, ss_ip);
                // insert_at_front(new_lru_node, head);
                printf("Create File done\n");
            }
            else
            {
                perror("[-]File create error");
                // exit(1);
                close(sock2);
                return -1;
            }

            // Send Ack to client that opeartion is successfull

            // close(sock2);
            do_backup_file(ss_num,input);
        }
        return 1;
    }

    else if (strncmp(input, "delete_file", strlen("delete_file")) == 0)
    {
        // search filepath in the trie
        // retireve ss_num and delete from ss_info and receive ack from ss
        // if found delete from trie
        // if present in lru delete from there
        // send ack to client

        // command = delete_file filepath
        // Get the file path from the command
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        int ss_num = search(root, file_path);

        if (ss_num <= 0)
        {
            printf("file not found!!!\n");
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", -1);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            return 1;
        }
        else if (ss_num > 0)
        {
            // Retrieve Storage Server information
            int ss_client_port = array_of_ss_info[ss_num].ss_client_port;
            int ss_nm_port = array_of_ss_info[ss_num].ss_nm_port;
            char *ss_ip = array_of_ss_info[ss_num].ss_ip;

            // Make a connection with the Storage Server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buffer[1024];

            // Create socket
            sock2 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr2, 0, sizeof(serv_addr2));
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = inet_addr(ss_ip);
            serv_addr2.sin_port = htons(ss_nm_port);

            // Connect to Storage Server
            if (connect(sock2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }
            printf("Connected to Storage Server %d\n", ss_num);

            // Send input to Storage Server
            if (send(sock2, input, strlen(input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive Ack from Storage Server
            int ack;
            char buffer_ack[BUF_SIZE];
            if (recv(sock2, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Convert received acknowledgment to an integer
            sscanf(buffer_ack, "%d", &ack);

            close(sock2);

            // Send the same acknowledgment to the client
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", ack);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            if (ack >= 0)
            {
                printf("Deleted File successfully!\n");
                // Acknowledgment received successfully: delete the file path from the trie
                delete_node(root, file_path);

                // Search and delete from the LRU cache
                lru_node *deleted_node = delete_lru_node(file_path, head);
                free(deleted_node);
            }
            else
            {
                printf("file not found!!!\n");
                // perror("[-]File delete error");
                // exit(1);
            }
            do_backup_file(ss_num,input);
            close(sock2);
        }
        return 1;
    }
    else if (strncmp(input, "copy_file", strlen("copy_file")) == 0)
    {
        // command: copy_file old_filepath new_folderpath
        char temp[1024];
        strcpy(temp, input);

        char *old_filepath;
        char *new_folderpath;

        // Tokenize the input to extract old_filepath and new_folderpath
        old_filepath = strtok(temp, " ");
        old_filepath = strtok(NULL, " ");
        new_folderpath = strtok(NULL, " ");

        char *filename = strrchr(old_filepath, '/');
        if (filename == NULL)
        {
            // Handle the case when there is no '/'
            filename = old_filepath;
        }
        else
        {
            // Move to the next character after '/'
            filename++;
        }

        // Concatenate filename with new_folderpath to get the new file path
        char new_filepath[1024];
        snprintf(new_filepath, sizeof(new_filepath), "%s/%s", new_folderpath, filename);

        // Check if the file is in the LRU cache
        lru_node *node_in_cache_1 = find_and_return(old_filepath, head);
        lru_node *node_in_cache_2 = find_and_return(new_folderpath, head);
        int ss_num_1;
        int ss_num_2;
        int ss_client_port_1, ss_client_port_2;
        int ss_nm_port_1, ss_nm_port_2;
        char *ss_ip_1;
        char *ss_ip_2;
        int port_1, port_2;
        // char ss_ip_3[21];
        // char ss_ip_4[21];

        if (node_in_cache_1 == NULL)
        {
            // printf("not in cache\n");
            // Search in trie
            ss_num_1 = search(root, old_filepath);
            // printf("ss_num_1: %d\n", ss_num_1);

            if (ss_num_1 == 0)
            {
                // If file not found in trie, send -1 as acknowledgment to the client
                printf("File path not found\n");
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }

            else
            {
                // printf("found in trie\n");
                // File found in trie
                // Retrieve Storage Server information
                ss_client_port_1 = array_of_ss_info[ss_num_1].ss_client_port;
                ss_nm_port_1 = array_of_ss_info[ss_num_1].ss_nm_port;
                ss_ip_1 = array_of_ss_info[ss_num_1].ss_ip;
            }
        }
        // File found in the LRU cache
        else
        {
            port_1 = node_in_cache_1->storage_server_port_for_client;
            strcpy(ss_ip_1, node_in_cache_1->storage_server_ip);
        }

        if (node_in_cache_2 == NULL)
        {
            // printf("not in cache\n");
            // Search in trie
            ss_num_2 = search(root, new_folderpath);
            // printf("ss_num_2: %d\n", ss_num_2);

            if (ss_num_2 == 0)
            {
                // If file not found in trie, send -1 as acknowledgment to the client
                printf("Folder path (Destination) not found\n");
                char send_details_to_client_1[BUF_SIZE];
                sprintf(send_details_to_client_1, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client_1, strlen(send_details_to_client_1), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }
            else
            {
                // printf("found in trie\n");
                // File found in trie
                //  Retrieve Storage Server information
                ss_client_port_2 = array_of_ss_info[ss_num_2].ss_client_port;
                ss_nm_port_2 = array_of_ss_info[ss_num_2].ss_nm_port;
                ss_ip_2 = array_of_ss_info[ss_num_2].ss_ip;
            }
        }
        // File found in the LRU cache
        else
        {
            // Retrieve Storage Server information
            port_2 = node_in_cache_2->storage_server_port_for_client;
            strcpy(ss_ip_2, node_in_cache_2->storage_server_ip);
        }
        if (ss_num_1 == ss_num_2)
        {
            // Make a connection with the Storage Server
            int sock1;
            struct sockaddr_in serv_addr1;
            char buffer1[BUF_SIZE];

            // Create socket
            sock1 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock1 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr1, 0, sizeof(serv_addr1));
            serv_addr1.sin_family = AF_INET;
            serv_addr1.sin_addr.s_addr = inet_addr(ss_ip_1);
            serv_addr1.sin_port = htons(ss_nm_port_1);

            // Connect to Storage Server
            if (connect(sock1, (struct sockaddr *)&serv_addr1, sizeof(serv_addr1)) == -1)
            {
                perror("connect() error");
                exit(1);
            }
            printf("Storage Server %d connected\n", ss_num_1);

            // Send input and "same" to Storage Server
            char combined_input[BUF_SIZE];
            sprintf(combined_input, "%s same", input);

            if (send(sock1, combined_input, strlen(combined_input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive acknowledgment from Storage Server
            int ack;
            char buffer_ack[BUF_SIZE];
            if (recv(sock1, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Close the connection to Storage Server
            close(sock1);

            // Send the same acknowledgment to the client
            if (send(nm_sock_for_client, buffer_ack, strlen(buffer_ack), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Insert into trie
            insert(root, new_filepath, ss_num_2);
            // Insert into LRU
            // lru_node *new_lru_node = make_lru_node(new_filepath, ss_num_2, ss_client_port_2, ss_ip_2);
            // insert_at_front(new_lru_node, head);
        }
        else
        {
            printf("reached here!");
            int sock_ss1, sock_ss2;
            struct sockaddr_in serv_addr_ss1, serv_addr_ss2;
            char buffer_ss1[BUF_SIZE], buffer_ss2[BUF_SIZE];

            // Create sockets
            sock_ss1 = socket(AF_INET, SOCK_STREAM, 0);
            sock_ss2 = socket(AF_INET, SOCK_STREAM, 0);

            if (sock_ss1 == -1 || sock_ss2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            // Setup connection details for Storage Server 1
            memset(&serv_addr_ss1, 0, sizeof(serv_addr_ss1));
            serv_addr_ss1.sin_family = AF_INET;
            serv_addr_ss1.sin_addr.s_addr = inet_addr(ss_ip_1);
            serv_addr_ss1.sin_port = htons(ss_nm_port_1);

            // Setup connection details for Storage Server 2
            memset(&serv_addr_ss2, 0, sizeof(serv_addr_ss2));
            serv_addr_ss2.sin_family = AF_INET;
            serv_addr_ss2.sin_addr.s_addr = inet_addr(ss_ip_2);
            serv_addr_ss2.sin_port = htons(ss_nm_port_2);

            // Connect to Storage Server 1
            if (connect(sock_ss1, (struct sockaddr *)&serv_addr_ss1, sizeof(serv_addr_ss1)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            // Connect to Storage Server 2
            if (connect(sock_ss2, (struct sockaddr *)&serv_addr_ss2, sizeof(serv_addr_ss2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            printf("Connected to Storage Server %d and Storage Server %d\n", ss_num_1, ss_num_2);
            char input_to_ss1[BUF_SIZE];
            sprintf(input_to_ss1, "%s send", input);
            // Send input to Storage Server 1
            if (send(sock_ss1, input_to_ss1, strlen(input_to_ss1), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            char input_to_ss2[BUF_SIZE];
            sprintf(input_to_ss2, "%s receive", input);
            // Send input to Storage Server 1
            if (send(sock_ss2, input_to_ss2, strlen(input_to_ss2), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive "create_file filepath" command from Storage Server 1
            char create_file_command_ss1[BUF_SIZE];
            bzero(create_file_command_ss1, BUF_SIZE);
            if (recv(sock_ss1, create_file_command_ss1, sizeof(create_file_command_ss1), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Tokenize the "create_file filepath" command and store filepath
            char temp[BUF_SIZE];
            strcpy(temp, create_file_command_ss1);
            char *token_ss1 = strtok(temp, " ");
            token_ss1 = strtok(NULL, " ");

            char filepath_ss1[BUF_SIZE];
            strcpy(filepath_ss1, token_ss1);
            printf("filepath: %s\n", filepath_ss1);

            // Insert into trie
            insert(root, filepath_ss1, ss_num_2);

            // Insert into LRU
            // lru_node *new_lru_node = make_lru_node(filepath_ss1, ss_num_2, ss_client_port_2, ss_ip_2);
            // insert_at_front(new_lru_node, head);

            // Send "create_file filepath" command to Storage Server 2
            // printf("NM %s\n", create_file_command_ss1);
            if (send(sock_ss2, create_file_command_ss1, strlen(create_file_command_ss1), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            char acknowledge[BUF_SIZE];
            bzero(acknowledge, BUF_SIZE);
            if (recv(sock_ss2, acknowledge, sizeof(acknowledge), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }
            if (send(sock_ss1, acknowledge, strlen(acknowledge), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive "NUM_packets" from Storage Server 1
            int num_packets;
            char num_packets_ss1[BUF_SIZE];
            if (recv(sock_ss1, num_packets_ss1, sizeof(num_packets_ss1), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Send the received "NUM_packets" to Storage Server 2
            if (send(sock_ss2, num_packets_ss1, strlen(num_packets_ss1), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Recv ack from SS2 and send to SS1
            char num_packets_ack[BUF_SIZE];
            if (recv(sock_ss2, num_packets_ack, sizeof(num_packets_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Send ack to SS1
            if (send(sock_ss1, num_packets_ack, strlen(num_packets_ack), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // printf("num_packets: %s\n", num_packets_ss1);
            // Convert NUM_packets to an integer
            sscanf(num_packets_ss1, "%d", &num_packets);
            // printf("num_packets int: %d\n", num_packets);

            // While loop to receive and send data packets
            while (num_packets--)
            {
                // Receive data packets from Storage Server 1
                char data_packet_ss1[BUF_SIZE];
                bzero(data_packet_ss1, BUF_SIZE);
                if (recv(sock_ss1, data_packet_ss1, sizeof(data_packet_ss1), 0) < 0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("data received: %s", data_packet_ss1);
                // Send data packets to Storage Server 2
                if (send(sock_ss2, data_packet_ss1, strlen(data_packet_ss1), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
            }

            // Recv ack from SS2 and send to SS1 and client
            char ack_ss2_full[BUF_SIZE];
            if (recv(sock_ss2, ack_ss2_full, sizeof(ack_ss2_full), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Send ack to SS1
            if (send(sock_ss1, ack_ss2_full, strlen(ack_ss2_full), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Send ack to the client
            if (send(nm_sock_for_client, ack_ss2_full, strlen(ack_ss2_full), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            printf("Copy File Done\n");
            // Close connections
            // close(sock_ss1);
            // close(sock_ss2);
        }
        // return 1;
    }

    else if (strncmp(input, "create_folder", strlen("create_folder")) == 0)
    {
        // command: create_folder folderpath
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        // Checking if the folder already exists in the trie
        if (search(root, file_path) > 0)
        {
            // Send -1 acknowledgment to the client
            printf("file already exists!!!\n");

            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", -1);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            return 1;
        }
        else
        {
            // Extract parent directory path
            char dir_path[1024];
            strcpy(dir_path, file_path);

            char *remove_file_name = strrchr(dir_path, '/');
            if (remove_file_name != NULL)
                *remove_file_name = '\0';

            // Find Storage Server number
            int ss_num = search(root, dir_path);
            if (ss_num == 0)
            {
                printf("directory not found!!!\n");
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }
            // Retrieve Storage Server information
            int ss_client_port = array_of_ss_info[ss_num].ss_client_port;
            int ss_nm_port = array_of_ss_info[ss_num].ss_nm_port;
            char *ss_ip = array_of_ss_info[ss_num].ss_ip;

            // Make a connection with the Storage Server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buffer[1024];

            // Create socket
            sock2 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr2, 0, sizeof(serv_addr2));
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = inet_addr(ss_ip);
            serv_addr2.sin_port = htons(ss_nm_port);

            // Connect to Storage Server
            if (connect(sock2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            printf("Connected to Storage Server %d\n", ss_num);

            // Send input to Storage Server
            if (send(sock2, input, strlen(input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive Ack from Storage Server
            int ack;
            char buffer_ack[BUF_SIZE];
            if (recv(sock2, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Convert received acknowledgment to an integer
            sscanf(buffer_ack, "%d", &ack);

            close(sock2);
            // Send the same acknowledgment to the client
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", ack);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            if (ack >= 0)
            {
                // Insert into trie
                insert(root, file_path, ss_num);

                // Insert into LRU
                // lru_node *new_lru_node = make_lru_node(file_path, ss_num, ss_client_port, ss_ip);
                // insert_at_front(new_lru_node, head);
                printf("Create Folder done\n");
            }
            else
            {
                perror("[-]File creation error");
                exit(1);
            }
            do_backup_file(ss_num,input);
            close(sock2);
        }
        return 1;
    }
    else if (strncmp(input, "delete_folder", strlen("delete_folder")) == 0)
    {
        // command: delete_folder folderpath
        char temp[1024];
        strcpy(temp, input);
        char *file_path = strtok(temp, " ");
        file_path = strtok(NULL, " ");

        int ss_num = search(root, file_path);

        if (ss_num <= 0)
        {
            printf("file not found!!!\n");
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", -1);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            return 1;
        }
        else if (ss_num > 0)
        {
            // Retrieve Storage Server information
            int ss_client_port = array_of_ss_info[ss_num].ss_client_port;
            int ss_nm_port = array_of_ss_info[ss_num].ss_nm_port;
            char *ss_ip = array_of_ss_info[ss_num].ss_ip;

            // Make a connection with the Storage Server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buffer[1024];

            // Create socket
            sock2 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr2, 0, sizeof(serv_addr2));
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = inet_addr(ss_ip);
            serv_addr2.sin_port = htons(ss_nm_port);

            // Connect to Storage Server
            if (connect(sock2, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }
            printf("Connected to Storage Server %d\n", ss_num);

            // Send input to Storage Server
            if (send(sock2, input, strlen(input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive Ack from Storage Server
            int ack;
            char buffer_ack[BUF_SIZE];
            if (recv(sock2, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Convert received acknowledgment to an integer
            sscanf(buffer_ack, "%d", &ack);

            close(sock2);

            // Send the same acknowledgment to the client
            char send_details_to_client[BUF_SIZE];
            sprintf(send_details_to_client, "%d", ack);
            if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            if (ack >= 0)
            {
                // Acknowledgment received successfully: delete the file path from the trie
                delete_node(root, file_path);

                // Search and delete from the LRU cache
                lru_node *deleted_node = delete_lru_node(file_path, head);
                free(deleted_node);
                printf("Deleted Folder successfully!\n");
            }
            else
            {
                perror("[-]File delete error");
                exit(1);
            }
            do_backup_file(ss_num,input);
            close(sock2);
        }
        return 1;
    }
    else if (strncmp(input, "copy_folder", strlen("copy_folder")) == 0)
    {
        // command copy_folder folderpath new_folderpath
        char temp[1024];
        strcpy(temp, input);

        char *old_folderpath;
        char *new_folderpath;

        // Tokenize the input to extract old_folderpath and new_folderpath
        old_folderpath = strtok(temp, " ");
        old_folderpath = strtok(NULL, " ");
        new_folderpath = strtok(NULL, " ");

        // Check if the folder is in the LRU cache
        lru_node *node_in_cache_1 = find_and_return(old_folderpath, head);
        lru_node *node_in_cache_2 = find_and_return(new_folderpath, head);
        int ss_num_1;
        int ss_num_2;
        int ss_client_port_1, ss_client_port_2;
        int ss_nm_port_1, ss_nm_port_2;
        char *ss_ip_1;
        char *ss_ip_2;
        int port_1, port_2;

        // if (node_in_cache_1 == NULL)
        // {
            // Search in trie
            ss_num_1 = search(root, old_folderpath);

            if (ss_num_1 == 0)
            {
                // If folder not found in trie, send -1 as acknowledgment to the client
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }

            else
            {
                // Folder found in trie
                // Retrieve Storage Server information
                ss_client_port_1 = array_of_ss_info[ss_num_1].ss_client_port;
                ss_nm_port_1 = array_of_ss_info[ss_num_1].ss_nm_port;
                ss_ip_1 = array_of_ss_info[ss_num_1].ss_ip;
            }
        // }
        // // Folder found in the LRU cache
        // else
        // {
        //     port_1 = node_in_cache_1->storage_server_port_for_client;
        //     strcpy(ss_ip_1, node_in_cache_1->storage_server_ip);
        // }

        // if (node_in_cache_2 == NULL)
        // {
        //     // Search in trie
            ss_num_2 = search(root, new_folderpath);

            if (ss_num_2 == 0)
            {
                // If folder not found in trie, send -1 as acknowledgment to the client
                char send_details_to_client[BUF_SIZE];
                sprintf(send_details_to_client, "%d", -1);
                if (send(nm_sock_for_client, send_details_to_client, strlen(send_details_to_client), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                return 1;
            }

            else
            {
                // Folder found in trie
                // Retrieve Storage Server information
                ss_client_port_2 = array_of_ss_info[ss_num_2].ss_client_port;
                ss_nm_port_2 = array_of_ss_info[ss_num_2].ss_nm_port;
                ss_ip_2 = array_of_ss_info[ss_num_2].ss_ip;
            }
        // }
        // // Folder found in the LRU cache
        // else
        // {
        //     // Retrieve Storage Server information
        //     port_2 = node_in_cache_2->storage_server_port_for_client;
        //     strcpy(ss_ip_2, node_in_cache_2->storage_server_ip);
        // }
        // printf("%d %d\n", ss_num_1, ss_num_2);

        if (ss_num_1 == ss_num_2)
        {
            // Make a connection with the Storage Server
            int sock1;
            struct sockaddr_in serv_addr1;
            char buffer1[BUF_SIZE];

            // Create socket
            sock1 = socket(AF_INET, SOCK_STREAM, 0);
            if (sock1 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            memset(&serv_addr1, 0, sizeof(serv_addr1));
            serv_addr1.sin_family = AF_INET;
            serv_addr1.sin_addr.s_addr = inet_addr(ss_ip_1);
            serv_addr1.sin_port = htons(ss_nm_port_1);

            // Connect to Storage Server
            if (connect(sock1, (struct sockaddr *)&serv_addr1, sizeof(serv_addr1)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            // Send input and "same" to Storage Server
            char combined_input[BUF_SIZE];
            sprintf(combined_input, "%s same", input);
            if (send(sock1, combined_input, strlen(combined_input), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Receive acknowledgment from Storage Server
            int ack;
            char buffer_ack[BUF_SIZE];
            if (recv(sock1, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }

            // Close the connection to Storage Server

            // Send the same acknowledgment to the client
            if (send(nm_sock_for_client, buffer_ack, strlen(buffer_ack), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

             // Search for entries with old_folderpath as prefix
            search_and_insert(root, old_folderpath);

            // Remove the first character of all entries in result_strings and store them in a new array
            char updated_strings[MAX_STRINGS][MAX_STRING_LENGTH];
            for (int i = 0; i < result_count; i++)
                strcpy(updated_strings[i], result_strings[i] + 1);

            // Concatenate the updated strings with new_folderpath and insert into trie
            for (int i = 0; i < result_count; i++)
            {
                char combined_path[4 * MAX_STRING_LENGTH];
                sprintf(combined_path, "%s%s", new_folderpath, updated_strings[i]);
                // Insert combined_path into trie
                insert(root, combined_path, ss_num_2); 
            }
        }
        else
        {
            int sock_ss1, sock_ss2;
            struct sockaddr_in serv_addr_ss1, serv_addr_ss2;
            char buffer_ss1[BUF_SIZE], buffer_ss2[BUF_SIZE];

            // Create sockets
            sock_ss1 = socket(AF_INET, SOCK_STREAM, 0);
            sock_ss2 = socket(AF_INET, SOCK_STREAM, 0);

            if (sock_ss1 == -1 || sock_ss2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            // Setup connection details for Storage Server 1
            memset(&serv_addr_ss1, 0, sizeof(serv_addr_ss1));
            serv_addr_ss1.sin_family = AF_INET;
            serv_addr_ss1.sin_addr.s_addr = inet_addr(ss_ip_1);
            serv_addr_ss1.sin_port = htons(ss_nm_port_1);

            // Setup connection details for Storage Server 2
            memset(&serv_addr_ss2, 0, sizeof(serv_addr_ss2));
            serv_addr_ss2.sin_family = AF_INET;
            serv_addr_ss2.sin_addr.s_addr = inet_addr(ss_ip_2);
            serv_addr_ss2.sin_port = htons(ss_nm_port_2);

            // Connect to Storage Server 1
            if (connect(sock_ss1, (struct sockaddr *)&serv_addr_ss1, sizeof(serv_addr_ss1)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            // Connect to Storage Server 2
            if (connect(sock_ss2, (struct sockaddr *)&serv_addr_ss2, sizeof(serv_addr_ss2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }

            char input_to_ss1[BUF_SIZE];
            sprintf(input_to_ss1, "%s send", input);
            // Send input to Storage Server 1
            if (send(sock_ss1, input_to_ss1, strlen(input_to_ss1), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
            char input_to_ss2[BUF_SIZE];
            sprintf(input_to_ss2, "%s receive", input);
            // Send input to Storage Server 2
            if (send(sock_ss2, input_to_ss2, strlen(input_to_ss2), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Start the process
            char bufferFor1[BUF_SIZE], bufferFor2[BUF_SIZE];
            while (1)
            {
                bzero(bufferFor1, BUF_SIZE);
                if (recv(sock_ss1, bufferFor1, sizeof(bufferFor1), 0) < 0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("::::%s\n", bufferFor1);
                if (strcmp(bufferFor1, "__DONE__") == 0)
                {
                    if (send(sock_ss2, bufferFor1, sizeof(bufferFor1), 0) < 0)
                    {
                        perror("send() error");
                        exit(1);
                    }
                    break;
                }
                else if ((strncmp(bufferFor1, "create_folder", strlen("create_folder")) == 0) || (strncmp(bufferFor1, "create_file", strlen("create_file")) == 0))
                {
                    char temp[BUF_SIZE];
                    strcpy(temp, bufferFor1);
                    char *token_ss1 = strtok(temp, " ");
                    token_ss1 = strtok(NULL, " ");

                    char filepath_ss1[BUF_SIZE];
                    strcpy(filepath_ss1, token_ss1);
                    // printf("filepath: %s\n", filepath_ss1);

                    // Insert into trie
                    insert(root, filepath_ss1, ss_num_2);
                    // printf("FIEL %s NUM %d\n", filepath_ss1, ss_num_2);

                    // Insert into LRU
                    // lru_node *new_lru_node = make_lru_node(filepath_ss1, ss_num_2, ss_client_port_2, ss_ip_2);
                    // insert_at_front(new_lru_node, head);
                }

                if (send(sock_ss2, bufferFor1, strlen(bufferFor1), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
                // printf("NM %s\n", bufferFor1);

                // acknowledgements
                if (recv(sock_ss2, bufferFor2, sizeof(bufferFor2), 0) < 0)
                {
                    perror("recv() error");
                    exit(1);
                }
                if (send(sock_ss1, bufferFor2, strlen(bufferFor2), 0) < 0)
                {
                    perror("send() error");
                    exit(1);
                }
            }
            char ack_ss2_full[BUF_SIZE];
            bzero(ack_ss2_full, BUF_SIZE);
            if (recv(sock_ss2, ack_ss2_full, sizeof(ack_ss2_full), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }
            // printf("aa gaya\n");
            // Send ack to SS1
            if (send(sock_ss1, ack_ss2_full, strlen(ack_ss2_full), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }

            // Send ack to the client
            if (send(nm_sock_for_client, ack_ss2_full, strlen(ack_ss2_full), 0) < 0)
            {
                perror("send() error");
                exit(1);
            }
        }
        // return 1;
    }
    else
    {
        if (strncmp(input, "exit", strlen("exit")) == 0)
        {
            char buffer_ack[BUF_SIZE];
            if (recv(nm_sock_for_client, buffer_ack, sizeof(buffer_ack), 0) < 0)
            {
                perror("recv() error");
                exit(1);
            }
            close(nm_sock_for_client);
        }
        printf("Invaid Command\n");
        return 0;
    }
}