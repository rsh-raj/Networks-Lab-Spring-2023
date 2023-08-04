#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>

#include "auth.h"
#include "request.h"
#include "response.h"
#include "dictionary.h"
#include "common.h"
#include "template.h"

int check = 0;
extern int auth;
int cors = 0;
extern session_head *head_ptr;
#define HASHTABLE_SIZE 100
extern route *hashtable[HASHTABLE_SIZE];

int hash(char *s)
{
    int ans = 0;
    int p = 1;
    int m = 100;
    for (int i = 0; i < strlen(s); i++)
    {
        ans += (s[i] - 'a') * p;
        ans = ans % m;
        p *= 31;
    }
    if (ans < 0)
        ans = 0;
    return ans;
}

void init_hash_table()
{
    if (check == 0)
    {
        for (int i = 0; i < HASHTABLE_SIZE; i++)
        {
            hashtable[i] = NULL;
        }
        check = 1;
    }
}

void add_route(char *r, OUT (*fn_ptr)(IN), char **methods, int num_methods)
{
    init_hash_table();

    route *temp = (route *)malloc(sizeof(route));
    temp->num_methods = num_methods;
    for (int i = 0; i < num_methods; i++)
    {
        temp->methods[i] = (char *)malloc(strlen(methods[i]) + 1);
        strcpy(temp->methods[i], methods[i]);
    }

    // int index = hash(r);
    int index = 50;
    strcpy(temp->endpoint, r);

    temp->fn_ptr = fn_ptr;
    temp->next = hashtable[index];
    temp->login_required = 0;
    hashtable[index] = temp;
}

// frees the header of the request
void free_header_request(struct Header *header)
{
    if (header)
    {
        free_header_request(header->next);
        if (header->name)
            free(header->name);
        if (header->values)
            free(header->values);
        free(header);
    }
}

// function to flush the buffer
void resetBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = '\0';
    }
}

// function to print the header of the request
void print_request(struct Request *request)
{
    if (request)
    {
        printf("Method: %s\n", request->request_method);
        printf("Request-URI: %s\n", request->url);
        printf("HTTP-Version: %s\n", request->HTTP_version);
        puts("Headers:");
        struct Header *h;
        for (h = request->headers; h; h = h->next)
        {
            printf("%32s: %s\n", h->name, h->values);
        }
        if (request->enttity_body)
            printf("Body: %s\n", request->enttity_body);
    }
}

// modified comparator that also includes the dynamic urls
int strcmp_dynamic(char *end, Request *req)
{
    char *in_url = req->url;

    char *dyn = strstr(end, "<");
    if (!dyn)
        return strcmp(end, in_url);

    // dyn points to <

    int num_points = dyn - end;
    if (strncmp(end, in_url, num_points) != 0)
        return -1;

    // extracting the name and val of the parameter
    char *dyn_end = strstr(end, ">");
    // extract the parameter name
    char name[30];
    strncpy(name, dyn + 1, (int)(dyn_end - dyn - 1));
    name[dyn_end-dyn-1] = '\0';

    // even the dynamic part of in_url starts at num_points
    // getting the value of the parameter
    char *dyn_start = in_url + num_points;
    int dyn_len = strcspn(dyn_start, "/");

    if (dyn_len == 0)
        return -1;
    char *dyn_string = malloc(dyn_len + 1);
    memcpy(dyn_string, dyn_start, dyn_len);
    dyn_string[dyn_len] = '\0';

    // comparing the remaining part of the strings
    dyn_end += 1;
    dyn_start += dyn_len;

    // selecting the data type of the parameter
    // printf("Name is %s\n", name);
    char *first = strtok(name, ":");
    char *second = strtok(NULL, ":");
    // printf("first:%s second:%s dyn_str:%s\n", first, second, dyn_string);

    int answer;
    if ((answer = strcmp(dyn_end, dyn_start)) == 0)
    {
        // means the later part of the string are also same
        // hence push it in the dictionary
        if (!second)
        {
            // no data given
            req->query_params.insert(&req->query_params, first, dyn_string, 1);
        }
        else
        {
            // data type is given and is in first
            if (strcmp(first, "int") == 0)
            {
                int val = atoi(dyn_string);
                // printf("Val is %d\n", val);
                req->query_params.insert(&req->query_params, second, &val, 0);
            }
            else
                req->query_params.insert(&req->query_params, second, dyn_string, 1);
        }
    }

    free(dyn_string);
    return answer;
}

// returns the function pointer of the function to execute
OUT(*get_function(Request *req, int *status, int sock))
(IN)
{
    // char* (*func)(int *); get_function(char *end){
    char *end = req->url;
    // int index = hash(end);
    *status = 200;
    int index = 50;
    route *head = hashtable[index];

    // printf("Head end : %sx, in end : %sx, index : %d\n", head->endpoint, end, index);
    while (head)
    {
        if (strcmp_dynamic(head->endpoint, req) == 0)
        {
            if (auth && head->login_required)
            {
                // authentication needed at this endpoint
                *status = check_authentication(req, sock);
                if (*status != 200)
                    return NULL;
            }

            char *method = req->request_method;
            for (int i = 0; i < head->num_methods; i++)
            {
                if (strcmp(head->methods[i], method) == 0)
                    return head->fn_ptr;
            }

            perror("Requested method not supported on given endpoint\n");
            exit(EXIT_SUCCESS);
        }

        head = head->next;
    }

    *status = 404;
    return NULL;
}

char *get_content_type(char *path)
{
    char *extension = strrchr(path, '.');
    if (extension == NULL)
    {
        return strdup("text/*");
    }
    else if (strcmp(extension, ".pdf") == 0)
    {
        // file is a pdf
        return strdup("application/pdf");
    }
    else if (strcmp(extension, ".html") == 0)
    {
        // file is a html
        return strdup("text/html");
    }
    else if (strcmp(extension, ".jpg") == 0)
    {
        // file is a txt
        return strdup("image/jpeg");
    }
    else if (strcmp(extension, ".png") == 0)
    {
        // file is a txt
        return strdup("image/png");
    }
    else if (strcmp(extension, ".css") == 0)
    {
        // file is a txt
        return strdup("text/css");
    }
    else if (strcmp(extension, ".js") == 0)
    {
        // file is a txt
        return strdup("application/javascript");
    }
    else
    {
        // any other file
        return strdup("text/*");
    }
}

void search_and_send_file(int new_socket, char *path)
{
    struct stat st;
    int status_code = 200;
    int x = stat(path + 1, &st);

    // create response object
    struct Response *response = new_response();

    if (x == -1)
    {
        status_code = 404;
        path = "/templates/not_found.html";
        stat(path + 1, &st);
    }

    set_header_and_HTTPversion(status_code, response);
    struct Header *h = response->headers;
    while (h->next)
        h = h->next;

    char *content_length = malloc(100);
    sprintf(content_length, "%lld", st.st_size);

    h->next = malloc(sizeof(struct Header));
    h = h->next;
    h->name = strdup("Content-Length");
    h->values = strdup(content_length);
    h->next = malloc(sizeof(struct Header));
    h = h->next;
    h->name = strdup("Content-Type");
    h->values = get_content_type(path);
    h->next = NULL;

    send_response_header(new_socket, response);
    free_response(response);
    send_response_file(new_socket, path + 1);
}

void send_forbidden_info(int socket, int status)
{

    if (status == 302)
    {
        // redirection already done by user_loader
        return;
    }

    /// create the response object
    struct Response *response = new_response();

    set_header_and_HTTPversion(status, response);

    // change this to something more professional

    char forbid[200] = "Forbidden for you, chal nikal lawde !!";
    set_header(response, "Content-Length", "37");
    set_header(response, "Content-Type", "text/*");

    send_response_header(socket, response);
    free_response(response);

    // sending the content of the response
    send(socket, forbid, strlen(forbid), 0);
}

void *execute(void *ptr)
{
    int new_socket = *((int *)ptr);
    free(ptr);

    while (1)
    {
        struct Request *req = receiveHeader(new_socket);
        if (!req)
        {
            printf("Connection closed\n");

            close(new_socket);
            pthread_exit(0);
            return NULL;
        }

        print_request(req);
        // Dictionary d = get_json(req);
        // Dictionary d = get_form_data(req);
        // printf("We have the dictionary : \n\n");
        // print_dict(d);

        int status;
        // printf("ABC\n");

        OUT (*func)(IN) = get_function(req, &status, new_socket);
        char *out_pointer;
        // printf("ABC\n");

        if (!func && status == 404) //not found
        {
            // also search the file in the server
            // printf("ABC\n");

            search_and_send_file(new_socket, req->url);
        }

        // add authorization code
        else if (auth == 1 && status != 200)
        {   
            // authorizartion is on but status is not 200, means auth isue
            // means that there is some auth issue
            // printf("ABC\n");

            send_forbidden_info(new_socket, status);
        }
        else{
            // execute the handler
            out_pointer = (*func)(req, new_socket);
        }

        if(out_pointer != NULL){
            // send the message to the client
            // set the header and configure the body
            Response *res = new_response();
            res->body = strdup(out_pointer);
            send_response(res, new_socket);

            free_response(res);
        }

        free_request(req);
    }
    // send_response_file(new_socket, path);

    close(new_socket);
}

void *create_server(void *p)
{
    int port = *((int *)p);
    printf("Starting server at port %d...", port);

    int server_fd, new_socket;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address, client_address;

    server_address.sin_family = AF_INET;
    inet_aton("127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(port);
    printf("Server address: %s Port: %d\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int addrlen;
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("New connection accepted from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        pthread_t thread1;
        int *ptr = (int *)malloc(sizeof(int));
        *ptr = new_socket;
        pthread_create(&thread1, NULL, execute, (void *)ptr);
    }
}

void *clean_session(void *t)
{

    while (1)
    {
        time_t interval = *((time_t *)t);

        session_table *head = head_ptr->head;
        time_t current_time = time(NULL);
        session_table *prev = head;

        // printf("TABLE IS -----------------------------------------\n");
        while (head)
        {
            time_t time_diff = current_time - head->timestamp;
            // printf("%d %s %s\n", head->timestamp, head->session_token, head->user_info);
            if (time_diff >= 300)
            {
                // free this one
                if (prev == head)
                {
                    // first one
                    head_ptr->head = head->next;
                    prev = head->next;
                    session_table *nxt = head->next;
                    free(head->user_info);
                    free(head);
                    head = nxt;
                    continue;
                }

                else
                {
                    prev->next = head->next;
                    session_table *nxt = head->next;
                    free(head->user_info);
                    free(head);
                    head = nxt;
                    continue;
                }
            }

            prev = head;
            head = head->next;
        }

        sleep(300);
    }
}


void create_app(int port)
{
    init_hash_table();
    int time_diff = 30;
    pthread_t session_cleaner, server;
    pthread_create(&server, NULL, create_server, (void *)&port);
    if(auth) pthread_create(&session_cleaner, NULL, clean_session, (void *)&time_diff);

    pthread_join(server, NULL);
}
