#include "ss_tries.h"

//initialise ss_trie

ss_trie* ss_init()
{
    ss_trie* ss_root = (ss_trie*)malloc(sizeof(ss_trie));
    ss_root->is_end = 0;
    for (int i = 0; i < 128; i++)
    {
        ss_root->next[i] = NULL;
    }
    return ss_root;
}

//insert into ss_trie

void ss_insert(ss_trie* root, char* str)
{
    // sem_wait(&trie_lock);
    ss_trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        if (p->next[str[i]] == NULL)
        {
            ss_trie* temp = (ss_trie*)malloc(sizeof(ss_trie));
            temp->is_end = 0;
            for (int j = 0; j < 128; j++)
            {
                temp->next[j] = NULL;
            }
            p->next[str[i]] = temp;
        }
        p = p->next[str[i]];
    }
    p->is_end = 1;
    // init_rwlock(&(p->rwlock));
    // printf("inserted in ss trie %s\n",str);
    // sem_post(&trie_lock);
}

//search in ss_trie

int ss_search(ss_trie* root, char* str)
{
    // sem_wait(&trie_lock);
    if(str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = '\0';
    ss_trie* p = root;
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

//delete from ss_trie

void ss_delete_node(ss_trie* root, char* str)
{
    // sem_wait(&trie_lock);
    ss_trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        if (p->next[str[i]] == NULL)
        {
            return;
        }
        p = p->next[str[i]];
    }
    p->is_end = 0;
    // sem_post(&trie_lock);
}

//print all strings in ss_trie in lexicographical order
void ss_print_trie(ss_trie* root, char* prefix)
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
            ss_print_trie(root->next[i],temp);
        }
    }
}

void ss_print_all_strings_in_trie(ss_trie* root)
{
    // sem_wait(&trie_lock);
    ss_print_trie(root,"");
    // sem_post(&trie_lock);
}

// int main()
// {
//     int n;
//     scanf("%d",&n);
//     ss_trie* root = init();
//     for (int i = 0; i < n; i++)
//     {
//         char str[100];
//         scanf("%s",str);
//         insert(root,str,i+1);
//         int ss_num=search(root,str);
//         if(ss_num>0)
//         {
//             printf("found %s %d\n",str,ss_num);
//         }
//         else
//         {
//             printf("not found %s\n",str);
//         }
//     } 
//     print_all_strings_in_trie(root);
//     return 0;
// }
