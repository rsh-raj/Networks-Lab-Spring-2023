#include "request.h"
#include "response.h"
#include "dictionary.h"
#include "common.h"
#include "template.h"
#include "auth.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

extern int check;
session_head *head_ptr;
int auth = 0;

void LoginManager()
{
    auth = 1;
    head_ptr = (session_head *)malloc(sizeof(session_head));
    head_ptr->head = NULL;
}

void login_required(char *end)
{
    // int index = hash(end);
    int index = 50;
    route *head = hashtable[index];

    while (head)
    {
        if (strcmp(head->endpoint, end) == 0)
        {
            head->login_required = 1;
            break;
        }

        head = head->next;
    }

    return;
}

void gen_random_string(char *dest, int len)
{
    char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand(time(NULL));  
    int i, index;
    for (i = 0; i < len; i++)
    {
        index = rand() % 62;
        dest[i] = charset[index];
    }
    dest[i] = '\0'; 
}

char *generate_session_token(char *pk)
{
    session_table *entry = (session_table *)malloc(sizeof(session_table));

    // generate a random token and add to table
    gen_random_string(entry->session_token, 40);
    // strcpy(entry->session_token, gen_random_string());
    entry->user_info = malloc(strlen(pk) + 1);
    strcpy(entry->user_info, pk);

    entry->timestamp = time(NULL);
    entry->next = head_ptr->head;
    head_ptr->head = entry;

    // added to table
    // now return this token to the redirect function
    return entry->session_token;
}

// user implementable function that returns that  status code for the request
// int user_loader(Request *req, char *pk, int socket, int status)
// {   
//     // user can do anything with the status code
//     // redirect or display forbidden as per need

//     // one can use the pk to get the user from the db
//     // and then load the user info into the current_user dictinary in request struct

//     // populates current user in the req array
//     if (!pk || strcmp(pk, "adityachoudhary.01m@gmail.com") != 0)
//     {
//         char url_f[100];
//         strcpy(url_f, "/login");
//         strcat(url_f, "?redirect=");
//         strcat(url_f, req->url);
//         redirect(socket, url_f, NULL);

//         return 302;
//     }

//     else
//     {
//         // update the current_user dictionary
//         return 200;
//     }

//     // else{
//     //     redirect(socket, "/login", NULL);
//     //     return 302;
//     // }
// }

int check_authentication(Request *req, int sock)
{
    // get the cookie
    Header *h = req->headers;
    char *cookie;
    char temp[100];
    int status = 200;

    while (h)
    {
        if (strcmp(h->name, "Cookie") == 0)
        {
            strcpy(temp, h->values);
            char *a = strtok(temp, "=");
            cookie = strtok(NULL, "=");
            break;
        }

        h = h->next;
    }

    if (!h)
    {
        // no cookie
        return user_loader(req, NULL, sock, 403);; // forbidden
    }

    // get the pk corresponding to the cookie
    char *pk = NULL;
    session_table *st = head_ptr->head;
    while (st)
    {
        if (strcmp(st->session_token, cookie) == 0)
        {
            pk = st->user_info;
            st->timestamp = time(NULL);
            break;
        }

        st = st->next;
    }

    if(!st){
        // no such cookie is logged in
        return user_loader(req, pk, sock, 403);  // forbidden
    }


    return user_loader(req, pk, sock, status);
}