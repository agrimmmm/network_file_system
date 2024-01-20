#include "headers.h"

int check_input(char* input)
{
    int flag=0;
    if(strncmp(input,"read",strlen("read"))==0)
        flag=1;
    if(strncmp(input,"write",strlen("write"))==0)
        flag=1;
    if(strncmp(input,"retrieve",strlen("retrieve"))==0)
        flag=1;
    if(strncmp(input,"create_file",strlen("create_file"))==0)
        flag=1;
    if(strncmp(input,"delete_file",strlen("delete_file"))==0)
        flag=1;
    if(strncmp(input,"copy_file",strlen("copy_file"))==0)
        flag=1;
    if(strncmp(input,"create_folder",strlen("create_folder"))==0)
        flag=1;
    if(strncmp(input,"delete_folder",strlen("delete_folder"))==0)
        flag=1;
    if(strncmp(input,"copy_folder",strlen("copy_folder"))==0)
        flag=1;
    
    return flag;
}

//client program
int main()
{
    char *nm_ip = "127.0.0.1";    // ip address of Naming Server
    int nm_port = 4546;        // port number of Naming Server
    int sock;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    int read_cnt;

    // Clients initiate communication with the Naming Server (NM) to interact with the NFS

    // Create socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    // Initialize serv_addr
    memset(&serv_addr, 0, sizeof(serv_addr)); //init
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(nm_ip);
    serv_addr.sin_port = htons(nm_port);

    // Connect to server
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect() error");
        exit(1);
    }

    //receive ack from Naming Server
    //add timeout later
    if(recv(sock,buf,BUF_SIZE,0)<0)
    {
        perror("recv() error");
        exit(1);
    }
    printf("%s\n",buf);
    
    //First send action to Naming Server 
    //actions are read write retrieve information are actions on files
    //create delete copy are actions on files and directories

    //send action to Naming Server take input from user
    //for read: read filename
    //for write: write filename and then after receiving ack write data
    //for retrieve information: retrieve filename
    //for create: create_file filepath
    //for delete: delete_file filepth
    //for copy file: copy_file filepath to newfilepath
    //for create folder: create_folder folderpath
    //for delete folder: delete_folder folderpath
    //for copy folder: copy_folder folderpath to newfolderpath

    //for read write retrieve information connect to storage server
    //create delete copy receive ack from Naming Server

    while(1)
    {

        char input[BUF_SIZE];
        bzero(input,BUF_SIZE);
        printf("Enter action: \n");
        fgets(input,100,stdin);

        if(strncmp(input,"exit",strlen("exit"))==0)
        {
            //send to naming server
            if(send(sock,input,strlen(input),0)<0)
            {
                perror("send() error");
                exit(1);
            }
            //diconnect from naming server
            close(sock);
            break;
        }

        while(input[strlen(input)-1] == '\n')
        {
            input[strlen(input)-1] = '\0';
        }
        // printf("input: %s\n",input);
        // printf("strlen: %ld\n", strlen(input));

        if(check_input(input)!=0)
        {
            if(send(sock,input,strlen(input),0)<0)
            {
                perror("send() error");
                exit(1);
            }
        }
        printf("sent action to naming server: %s\n",input);       

        //if input is read write or retrieve information then receive port and ip of storage server
        //connect to storage server and send action to storage server

        if(strncmp(input,"read",strlen("read"))==0 || strncmp(input,"write",strlen("write"))==0 || strncmp(input,"retrieve",strlen("retrieve"))==0)
        {
            //receive port and ip of storage server
            bzero(buf,BUF_SIZE);
            if(recv(sock,buf,BUF_SIZE,0)<0)
            {
                perror("recv() error");
                exit(1);
            }
            //check if -1 is received then file doesnt exist and continue
            if(strcmp(buf,"-1")==0)
            {
                printf("File doesnt exist\n");
                continue;
            }

            
            char *server_ip = strtok(buf," ");
            char *server_port = strtok(NULL," ");
            printf("received naming server ip: %s \tport: %s\n",server_ip,server_port);

            //connect to storage server
            int sock2;
            struct sockaddr_in serv_addr2;
            char buf2[BUF_SIZE];

            // Create socket
            sock2 = socket(PF_INET, SOCK_STREAM, 0);
            if (sock2 == -1)
            {
                perror("socket() error");
                exit(1);
            }

            // Initialize serv_addr
            memset(&serv_addr2, 0, sizeof(serv_addr2)); //init
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_port = htons(atoi(server_port));
            serv_addr2.sin_addr.s_addr = inet_addr(server_ip);

            // Connect to server
            if(connect(sock2, (struct sockaddr*)&serv_addr2, sizeof(serv_addr2)) == -1)
            {
                perror("connect() error");
                exit(1);
            }
            printf("connected to storage server\n");

            //if input is read then receive data from storage server
            //if input is write then send data to storage server
            //if input is retrieve information then receive information from storage server

            if(strncmp(input,"read",strlen("read"))==0)
            {
                if(send(sock2,input,BUF_SIZE,0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
                // printf("sent read to storage server\n");
                //receive data from storage server
                //first receive number of packets as integer    
                char num_packets[BUF_SIZE];
                bzero(num_packets,BUF_SIZE);
                if(recv(sock2,num_packets,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                int num_packets_int = atoi(num_packets);
                bzero(buf2,BUF_SIZE);
                if(send(sock2,"ack",strlen("ack"),0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
                printf("reading file...\n");
                // recv data from storage server
                while(num_packets_int--)
                {
                    bzero(buf2,BUF_SIZE);
                    if(recv(sock2,buf2,BUF_SIZE,0)<0)
                    {
                        perror("recv() error");
                        exit(1);
                    }
                    printf("%s",buf2);
                }
                printf("\n");
                int ack = 1;
                bzero(buf2,BUF_SIZE);
                sprintf(buf2,"%d",ack);
                if(send(sock2,buf2,BUF_SIZE,0)<0)
                {
                    perror("send() error");
                    exit(1);
                }

                //send done ack to naming server
                bzero(buf,BUF_SIZE);
                strcpy(buf,"done");
                if(send(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
            }
            else if(strncmp(input,"write",strlen("write"))==0)
            {
                if(send(sock2,input,strlen(input),0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
                //receive OK from storage server
                bzero(buf2,BUF_SIZE);
                if(recv(sock2,buf2,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                //send data to storage server
                char data[BUF_SIZE];
                printf("Enter data to write in file: \n");
                //scan data until 2 enters continuously
                while(1)
                {
                    //send line by line
                    //assume that each line is less than 1024 characters
                    //also send \n at end of data
                    fgets(data,BUF_SIZE,stdin);
                    
                    if(send(sock2,data,strlen(data),0)<0)
                    {
                        perror("send() error");
                        exit(1);
                    }
                    
                    //if 2 enters then break
                    if(strcmp(data,"\n")==0)
                    {
                        break;
                    }
                    // bzero(buf2,BUF_SIZE);
                    // if(recv(sock2,buf2,BUF_SIZE,0)<0)
                    // {
                    //     perror("recv() error");
                    //     exit(1);
                    // }
                }
                //send ack to naming server
                bzero(buf,BUF_SIZE);
                strcpy(buf,"done");
                if(send(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
                // printf("done writing\n");
            }
            else if(strncmp(input,"retrieve",strlen("retrieve"))==0)
            {
                if(send(sock2,input,strlen(input),0)<0)
                {
                    perror("send() error");
                    exit(1);
                }
                //receive information from storage server
                
                bzero(buf2,BUF_SIZE);
                if(recv(sock2,buf2,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                if(strcmp(buf2,"-1")==0)
                {
                    printf("File doesnt exist\n");
                    continue;
                }
                else
                {
                    printf("details of file:\n");
                    printf("%s\n",buf2);
                }
                
                
            }

            //close connection with storage server
            // close(sock2);
            // printf("connection with storage server closed\n");
        }
        else
        {
            // printf("waiting for ack from naming server\n");
            bzero(buf,BUF_SIZE);
            if(strncmp(input,"create_file",strlen("create_file"))==0)
            {
                //receive ack from naming server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                
                //if ack is -1 then file cant be created as it already exists
                if(strcmp(buf,"-1")==0)
                {
                    printf("File already exists\n");
                    continue;
                }
                else
                {
                    printf("file created!!\n");
                }

                // printf("%s",buf);
            }
            else if(strncmp(input,"delete_file",strlen("delete_file"))==0)
            {
                //receive ack from naming server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                if(strcmp(buf,"-1")==0)
                {
                    printf("File can't be deleted\n");
                    continue;
                }
                else
                {
                    printf("file deleted successfully!!\n");
                }
                // printf("%s",buf);
            }
            else if(strncmp(input,"copy_file",strlen("copy_file"))==0)
            {
                //receive ack from naming server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
                if(strcmp(buf,"-1")==0)
                {
                    printf("File can't be copied\n");
                    continue;
                }
                else
                {
                    printf("file copied successfully!!\n");
                }
            }
            else if(strncmp(input,"create_folder",strlen("create_folder"))==0)
            {
                //receive ack from naming server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                // printf("%s",buf);
                if(strcmp(buf,"-1")==0)
                {
                    printf("Folder can't be created\n");
                    continue;
                }
                else
                {
                    printf("folder created successfully!!\n");
                }
            }
            else if(strncmp(input,"delete_folder",strlen("delete_folder"))==0)
            {
                //receive ack from naming server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                if(strcmp(buf,"-1")==0)
                {
                    printf("Folder can't be deleted\n");
                    continue;
                }
                else
                {
                    printf("folder deleted successfully!!\n");
                }
                // printf("%s",buf);
            }
            else if(strncmp(input,"copy_folder",strlen("copy_folder"))==0)
            {
                //receive ack from naming server
                if(recv(sock,buf,BUF_SIZE,0)<0)
                {
                    perror("recv() error");
                    exit(1);
                }
                if(strcmp(buf,"-1")==0)
                {
                    printf("Folder can't be copied\n");
                    continue;
                }
                else
                {
                    printf("folder copied successfully!!\n");
                }
                // printf("%s",buf);
            }
            else
            {
                // if(recv(sock,buf,BUF_SIZE,0)<0)
                // {
                //     perror("recv() error");
                //     exit(1);
                // }
                printf("Invalid action !!!\n");
            }
        
        }

    }

    printf("Exiting...\n");
    close(sock);

}