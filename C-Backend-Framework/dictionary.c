#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"

int isBool(char *s){
    if(strcmp(s,"true") == 0 || strcmp(s, "false") == 0) return 1;
    return 0;
}

int isNull(char *s){
    if(strcmp(s,"NULL") == 0 || strcmp(s, "null") == 0) return 1;
    return 0;
}

void insert_dict(Dictionary *dictionary, char *key, void *value, int isString)
{   
    if(!key || !value) return;
    
    node *head = dictionary->head;
    node *temp = (node *)malloc(sizeof(node));
    temp->isString = isString;
    temp->key = (char *)malloc(strlen(key) + 1);
    strcpy(temp->key, key);

    if (isString)
    {
        char *cnt = (char *)value;
        temp->val = (char *)malloc(strlen(cnt) + 1);
        strcpy(temp->val, cnt);
    }
    else
    {

        if (isBool((char *)value) || isNull((char *)value))
        {
            temp->val = malloc(10);
            strcpy(temp->val, (char *)value);
            // printf("Obtained is %s\n", (char *)value);
        }
        else
        {
            int *cnt = (int *)value;
            temp->val = (int *)malloc(sizeof(int));
            *((int *)temp->val) = *((int *)value);
        }
    }

    temp->next = head;
    dictionary->head = temp;
}

void *search_dict(Dictionary *dictionary, char *key)
{
    node *head = dictionary->head;
    while (head)
    {
        if (strcmp(key, head->key) == 0)
        {
            return head->val;
        }

        head = head->next;
    }

    return NULL;
}

Dictionary init_dict()
{
    Dictionary dictionary;
    dictionary.insert = insert_dict;
    dictionary.search = search_dict;
    dictionary.head = NULL;
    return dictionary;
}

void free_dict_node(node *h)
{
    if (h)
    {
        if (h->key)
            free(h->key);
        if (h->val)
            free(h->val);
    }
}

void destroy_dictionary(Dictionary *dict)
{   
    if(!dict->head) return;
    node *head = dict->head;
    while (head)
    {
        free_dict_node(head);
        node *temp = head->next;
        free(head);
        head = temp;
    }

    dict->head = NULL;
}

void print_dict(Dictionary d)
{
    node *head = d.head;
    printf("{\n");
    while (head)
    {
        printf("  ");
        if (head->isString)
        {
            printf("\"%s\" : \"%s\"", head->key, (char *)head->val);
        }
        else
        {
            if (isNull((char *)head->val) || isBool((char *)head->val))
                printf("\"%s\" : %s", head->key, ((char *)head->val));
            else
                printf("\"%s\" : %d", head->key, *((int *)head->val));
        }

        if (head->next)
            printf(",\n");
        else
            printf("\n");

        head = head->next;
    }
    printf("}\n");
}

char *toString_dict(node *head, int isList)
{   
    char *output = malloc(10000);
    strcpy(output, "{\n");
    while (head)
    {
        // output = realloc(output, strlen(output) + 7 + strlen(head->key));
        strcat(output, "  \"");
        strcat(output, head->key);
        strcat(output, "\"");
        strcat(output, ": ");

        // adding the value to the dictionary
        if (head->isString)
        {
            // output = realloc(output, strlen(output) + 3 + strlen(head->val));
            strcat(output, "\"");
            strcat(output, head->val);
            strcat(output, "\"");
        }
        else
        {
            // output = realloc(output, strlen(output) + 20);
            if (isNull((char *)head->val) || isBool((char *)head->val))
            {
                strcat(output, (char *)head->val);
            }
            else{
                char str[10];
                sprintf(str, "%d", *((int *)head->val));
                strcat(output, str);
            }
        }

        if (head->next)
            strcat(output, ",\n");
        else
            strcat(output, "\n");

        head = head->next;
    }
    if(isList) strcat(output, " ");
    strcat(output, "}");

    return output;
}

char *convert_dict_to_string(Dictionary *d, int isList, int size)
{
    char temp[10000];
    temp[0] = '\0';
    if (isList == 0)
    {   
        node *head = d->head;
        char *output = toString_dict(head, isList);
        strcat(temp, output);
        free(output);
        destroy_dictionary(d);
    }
    else
    {
        strcpy(temp, "[\n");
        for (int i = 0; i < size; i++)
        {
            node *head = d[i].head;
            strcat(temp, " ");
            char *tput = toString_dict(head, isList);
            strcat(temp, tput);
            free(tput);

            if(i < size-1) strcat(temp, ",\n");
            else strcat(temp, "\n");
            destroy_dictionary(&d[i]);

        }
        
        strcat(temp, "]");
    }

    char *output = malloc(strlen(temp)+1);
    strcpy(output, temp);

    return output;
}


// int main()
// {
//     Dictionary d = init_dict();
//     Dictionary c = init_dict();
//     Dictionary m[2];

//     c.insert(&c, "name", "Akshat", 1);
//     int y = 21;
//     c.insert(&c, "age", &y, 0);
//     c.insert(&c, "girl friends", "true", 1);

//     d.insert(&d, "name", "aditya", 1);
//     int x = 20;
//     d.insert(&d, "age", &x, 0);
//     void *s = d.search(&d, "name");
//     void *p = d.search(&d, "age");
//     printf("Name obtained is %s and age is %d\n", (char *)s, *((int *)(p)));

//     d.insert(&d, "girl friends", NULL, 0);
//     // print_dict(d);
//     m[0] = d;
//     m[1] = c;
//     char *out = convert_dict_to_string(m,1,2);
//     printf("%s\n", out);
//     // destroy_dictionary(&d);
//     return 0;
// }
