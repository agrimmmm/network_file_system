#include "ss_info.h"
#include "headers.h"

extern sem_t ss_info_lock;

ss_info* array_of_ss_info;

void init_ss_info()
{
    array_of_ss_info = (ss_info*)malloc(sizeof(ss_info)*NUM_STORAGE_SERVERS);
    for(int i=0; i<NUM_STORAGE_SERVERS; i++)
    {
        array_of_ss_info[i].ss_num = -1;
        array_of_ss_info[i].ss_ip = (char*)calloc(sizeof(char), 21);
    }
}

void insert_ss_info(int ss_num, int ss_client_port, int ss_nm_port, char* ss_ip)
{
    sem_wait(&ss_info_lock);
    if(array_of_ss_info[ss_num].ss_num != -1)
    {
        printf("Storage Server with number %d already exists\n", ss_num);
        return;
    }
    array_of_ss_info[ss_num].ss_num = ss_num;
    array_of_ss_info[ss_num].ss_client_port = ss_client_port;
    array_of_ss_info[ss_num].ss_nm_port = ss_nm_port;
    strcpy(array_of_ss_info[ss_num].ss_ip, ss_ip);
    sem_post(&ss_info_lock);
}

void delete_ss(int ss_num)
{
    array_of_ss_info[ss_num].ss_num = -1;
}

void print_ss_info()
{
    for(int i=0;i<NUM_STORAGE_SERVERS;i++)
    {
        if(array_of_ss_info[i].ss_num != -1)
            printf("%d %d %d %s\n", array_of_ss_info[i].ss_num, array_of_ss_info[i].ss_client_port, array_of_ss_info[i].ss_nm_port, array_of_ss_info[i].ss_ip);
    }
}

// int main()
// {
//     init_ss_info();
//     int n;
//     scanf("%d", &n);
//     for(int i=0;i<n;i++)
//     {
//         int ss_num, ss_client_port, ss_nm_port;
//         char ss_ip[21];
//         scanf("%d %d %d %s", &ss_num, &ss_client_port, &ss_nm_port, ss_ip);
//         insert_ss_info(ss_num, ss_client_port, ss_nm_port, ss_ip);
//     }
//     scanf("%d", &n);
//     for(int i=0;i<n;i++)
//     {
//         int ss_num;
//         scanf("%d", &ss_num);
//         delete_ss(ss_num);
//     }
//     print_ss_info();
//     return 0;
// }


