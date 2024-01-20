#include "lru.h"
#include "headers.h"

extern sem_t lru_lock;

lru_head* head;

lru_node* find_and_return(char* filepath, lru_head* head)
{
    sem_wait(&lru_lock);
    lru_node* temp = head->front;
    while(temp != NULL)
    {
        if(strcmp(temp->filepath, filepath) == 0)
        {
            sem_post(&lru_lock);
            return temp;
        }
        temp = temp->next;
    }
    sem_post(&lru_lock);
    return NULL;
}

lru_head* init_lru()
{
    lru_head* head = (lru_head*)malloc(sizeof(lru_head));
    head->num_nodes = 0;
    head->front = NULL;
    head->rear = NULL;
    return head;
}

lru_node* make_lru_node(char* filepath, int storage_server_num, int storage_server_port_for_client, char* storage_server_ip)
{
    sem_wait(&lru_lock);
    lru_node* node = (lru_node*)malloc(sizeof(lru_node));
    node->filepath = (char*)calloc(sizeof(char), strlen(filepath));
    strcpy(node->filepath, filepath);
    node->storage_server_num = storage_server_num;
    node->storage_server_port_for_client = storage_server_port_for_client;
    node->storage_server_ip = (char*)calloc(sizeof(char), strlen(storage_server_ip));
    strcpy(node->storage_server_ip, storage_server_ip);
    node->next = NULL;
    node->prev = NULL;
    sem_post(&lru_lock);
    return node;
}

void delete_last_node(lru_head* head)
{
    sem_wait(&lru_lock);
    if(head->num_nodes == 0)
    {
        sem_post(&lru_lock);
        return;
    }
    if(head->num_nodes == 1)
    {
        head->front = NULL;
        head->rear = NULL;
        head->num_nodes--;
        sem_post(&lru_lock);
        return;
    }
    lru_node* temp = head->rear;
    head->rear = head->rear->prev;
    head->rear->next = NULL;
    head->num_nodes--;
    sem_post(&lru_lock);
    return;
}

void insert_at_front(lru_node* node, lru_head* head)
{
    sem_wait(&lru_lock);
    if(head->num_nodes == LRU_CACHE_SIZE)
    {
        lru_node* temp = head->rear;
        head->rear = head->rear->prev;
        head->rear->next = NULL;
        head->num_nodes--;
    }

    if(head->num_nodes == 0)
    {
        head->front = node;
        head->rear = node;
        head->num_nodes++;
        sem_post(&lru_lock);
        return;
    }
    node->next = head->front;
    node->prev = NULL;
    node->next->prev = node;
    head->front = node;
    head->num_nodes++;
    sem_post(&lru_lock);
    return;
}

lru_node* delete_lru_node(char* filepath, lru_head* head)
{
    sem_wait(&lru_lock);
    lru_node* return_node = NULL;
    if(head->num_nodes == 0)
    {
        sem_post(&lru_lock);
        return NULL;
    }
    if(head->num_nodes == 1)
    {
        return_node = head->front;
        head->front = NULL;
        head->rear = NULL;
        head->num_nodes--;
        sem_post(&lru_lock);
        return return_node;
    }
    if(strcmp(head->front->filepath, filepath) == 0)
    {
        return_node = head->front;
        head->front = head->front->next;
        head->front->prev = NULL;
        head->num_nodes--;
        sem_post(&lru_lock);
        return return_node;
    }
    lru_node* temp = head->front;
    while(temp->next != NULL)
    {
        if(strcmp(temp->next->filepath, filepath) == 0)
        {
            return_node = temp->next;
            temp->next = temp->next->next;
            if(temp->next != NULL)
                temp->next->prev = temp;
            head->num_nodes--;
            sem_post(&lru_lock);
            return return_node;
        }
        temp = temp->next;
    }
    sem_post(&lru_lock);
    return NULL;

}

//lock handling lite

void shift_node_to_front(char* filepath, lru_head* head)
{
    if(head->num_nodes == 0)
        return;
    if(head->num_nodes == 1)
        return;
    if(strcmp(head->front->filepath, filepath) == 0)
        return;
    if(find_and_return(filepath, head) == NULL)
        return;
    lru_node* deleted_node = delete_lru_node(filepath, head);
    insert_at_front(deleted_node, head);
}

void print_lru(lru_head* head)
{
    lru_node* temp = head->front;
    while(temp != NULL)
    {
        printf("%s %d %d %s\n", temp->filepath, temp->storage_server_num, temp->storage_server_port_for_client, temp->storage_server_ip);
        temp = temp->next;
    }
}

// int main()
// {
//     //to test lru working
//     head = init_lru();
//     int n;
//     scanf("%d", &n);
//     for(int i = 0; i < n; i++)
//     {
//         char filepath[100];
//         int storage_server_num;
//         int storage_server_port;
//         char storage_server_ip[100];
//         scanf("%s %d %d %s", filepath, &storage_server_num, &storage_server_port, storage_server_ip);
//         lru_node* node = make_lru_node(filepath, storage_server_num, storage_server_port, storage_server_ip);
//         insert_at_front(node, head);
//     }
//     print_lru(head);
//     printf("\n");
//     delete_last_node(head);
//     print_lru(head);
//     printf("\n");
//     //search a filepath
//     int search_num;
//     scanf("%d", &search_num);
//     while(search_num--)
//     {
//         char filepath[100];
//         scanf("%s", filepath);
//         lru_node* node = find_and_return(filepath, head);
//         if(node != NULL)
//         {
//             printf("%s %d %d %s\n", node->filepath, node->storage_server_num, node->storage_server_port, node->storage_server_ip);
//         }
//         else
//         {
//             printf("Not found\n");
//         }
//     }
//     delete_last_node(head);
//     print_lru(head);
//     printf("\n");
//     //shift to front
//     int shift_num;
//     scanf("%d", &shift_num);
//     while(shift_num--)
//     {
//         char filepath[100];
//         scanf("%s", filepath);
//         shift_node_to_front(filepath, head);
//         print_lru(head);
//     }
//     printf("\n");
// }    