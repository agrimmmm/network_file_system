#include "tries.h"
#include "headers.h"

sem_t trie_lock;

extern trie* root;
extern char result_strings[MAX_STRINGS][MAX_STRING_LENGTH];
extern int result_count;

//initialise trie

trie* init()
{
    trie* root = (trie*)malloc(sizeof(trie));
    root->is_end = 0;
    for (int i = 0; i < 128; i++)
    {
        root->next[i] = NULL;
    }
    return root;
}

//insert into trie

void insert(trie* root, char* str, int server_num)
{
    sem_wait(&trie_lock);
    trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        if (p->next[str[i]] == NULL)
        {
            trie* temp = (trie*)malloc(sizeof(trie));
            temp->is_end = 0;
            for (int j = 0; j < 128; j++)
            {
                temp->next[j] = NULL;
            }
            p->next[str[i]] = temp;
        }
        p = p->next[str[i]];
    }
    p->is_end = server_num;
    init_rwlock(&(p->rwlock));
    printf("inserted %s\n",str);
    sem_post(&trie_lock);
}

//search in trie

int search(trie* root, char* str)
{
    // sem_wait(&trie_lock);
    if(str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = '\0';
    trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        
        if (p->next[str[i]] == NULL)
        {
            // sem_post(&trie_lock);
            return 0;
        }
        p = p->next[str[i]];
    }
    if (p->is_end > 0)
    {
        // sem_post(&trie_lock);
        return p->is_end;
    }
    // sem_post(&trie_lock);
    return 0;
}

rwlock_t* find_rwlock(trie* root, char* str)
{
    sem_wait(&trie_lock);
    if(str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = '\0';
    trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        
        if (p->next[str[i]] == NULL)
        {
            sem_post(&trie_lock);
            return NULL;
        }
        p = p->next[str[i]];
    }
    if (p->is_end > 0)
    {
        sem_post(&trie_lock);
        return &(p->rwlock);
    }
    sem_post(&trie_lock);
    return NULL;
}

//delete from trie

void delete_node(trie* root, char* str)
{
    sem_wait(&trie_lock);
    trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        if (p->next[str[i]] == NULL)
        {
            return;
        }
        p = p->next[str[i]];
    }
    p->is_end = 0;
    sem_post(&trie_lock);
}

//print all strings in trie in lexicographical order
void print_trie(trie* root, char* prefix)
{
    for(int i=0;i<128;i++)
    {
        if(root->next[i]!=NULL)
        {
            char* temp = (char*)malloc(sizeof(char)*(strlen(prefix)+2));
            strcpy(temp,prefix);
            temp[strlen(prefix)] = i;
            temp[strlen(prefix)+1] = '\0';
            if(root->next[i]->is_end>0)
            {
                printf("%s\n",temp);
            }
            print_trie(root->next[i],temp);
        }
    }
}

void print_all_strings_in_trie(trie* root)
{
    // sem_wait(&trie_lock);
    print_trie(root,"");
    // sem_post(&trie_lock);
}

void collect_strings_with_prefix(trie *node, char *current_prefix)
{
    if (node == NULL)
        return;

    if (node->is_end > 0)
    {
        // Copy the string to the 2D array
        strcpy(result_strings[result_count], current_prefix);
        result_count++;
    }

    // Recursively traverse the children of the current node
    for (int i = 0; i < 128; i++)
    {
        if (node->next[i] != NULL)
        {
            char *temp = (char *)malloc(sizeof(char) * (strlen(current_prefix) + 2));
            strcpy(temp, current_prefix);
            temp[strlen(current_prefix)] = i;
            temp[strlen(current_prefix) + 1] = '\0';

            collect_strings_with_prefix(node->next[i], temp);
        }
    }
}

void search_and_insert(trie *root, char *prefix)
{
    trie *p = root;
    int len = strlen(prefix);

    for (int i = 0; i < len; i++)
    {
        if (p->next[prefix[i]] == NULL)
        {
            sem_post(&trie_lock);
            return;
        }
        p = p->next[prefix[i]];
    }

    collect_strings_with_prefix(p, prefix);
}

void print_result_strings()
{
    for (int i = 0; i < result_count; i++)
        printf("%s\n", result_strings[i]);
}

// int main()
// {
//     int n;
//     char input[MAX_STRING_LENGTH];

//     root = init();

//     printf("Enter the number of strings to insert: ");
//     scanf("%d", &n);

//     for (int i = 0; i < n; i++)
//     {
//         printf("Enter string %d: ", i + 1);
//         scanf("%s", input);
//         insert(root, input, i + 1);
//     }

//     printf("Enter the prefix to search: ");
//     scanf("%s", input);

//     search_and_insert(root, input);

//     printf("Strings with the prefix '%s':\n", input);
//     print_result_strings();

//     return 0;
// }
