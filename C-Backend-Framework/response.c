#include "common.h"
#include "response.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>

extern int cors;

// frees the response data structures
void free_response(struct Response *response)
{
    if (response)
    {
        if (response->headers)
            free_header_request(response->headers);
        if(response->body)
            free(response->body);
        free(response);
    }
}


char *modifydate(int changeday, struct tm tm)
{
    // time_t t = time(NULL);
    // struct tm tm = *localtime(&t);
    tm.tm_mday += changeday;
    mktime(&tm);
    char buf[50];
    strcpy(buf, asctime(&tm));

    buf[strlen(buf) - 1] = '\0';
    // printf("%s\n", buf);

    char *temp[10];
    temp[0] = strtok(buf, " ");
    temp[1] = strtok(NULL, " ");
    temp[2] = strtok(NULL, " ");
    temp[3] = strtok(NULL, " ");
    temp[4] = strtok(NULL, " ");

    // Now HTTP formatting
    char *final = (char *)malloc(100 * sizeof(char));
    strcpy(final, temp[0]);
    strcat(final, ", ");
    strcat(final, temp[2]);
    strcat(final, " ");
    strcat(final, temp[1]);
    strcat(final, " ");
    strcat(final, temp[4]);
    strcat(final, " ");
    strcat(final, temp[3]);
    strcat(final, " ");
    strcat(final, "IST");

    // printf("Final date : %s\n", final);
    // returns a malloced pointer
    return final;
}

void send_response_header(int client_sockfd, struct Response *response)
{
    char status_code[4];
    sprintf(status_code, "%d", response->status_code);

    int length_to_allocate = 0;
    length_to_allocate += strlen(response->HTTP_version) + strlen(status_code) + strlen(response->status_message) + 4;

    Header *head = response->headers;
    int header_len = 0;
    while(head){
        header_len += (strlen(head->name) + strlen(head->values) + 4);
        head = head->next;
    }

    length_to_allocate += header_len + 10;

    char *responseString = malloc(length_to_allocate);
    
    strcpy(responseString, response->HTTP_version);
    strcat(responseString, " ");

    strcat(responseString, status_code);

    // version + 1 + status_code + 1 + status_message + 2(\r\n)
    // for each header space needed, h-name h-values + 2( :) + 2(\r\n)
    // last space +3 \r\n\0
    strcat(responseString, " ");
    strcat(responseString, response->status_message);
    strcat(responseString, "\r\n");

    struct Header *h;
    for (h = response->headers; h; h = h->next)
    {
        // char *temp = (char *)realloc(responseString, strlen(responseString) + strlen(h->name) + strlen(h->values) + 5);
        // responseString = temp;

        strcat(responseString, h->name);
        strcat(responseString, ": ");
        strcat(responseString, h->values);
        strcat(responseString, "\r\n");
    }

    strcat(responseString, "\r\n");
    printf("FOLLOWING RESPONSE STRING HAS BEEN SENT TO CLIENT:\n");
    printf("%s\n", responseString);
    send(client_sockfd, responseString, strlen(responseString), 0); // send the header and HTTP version

    free(responseString);
}

void send_response_file(int new_socket, char *url)
{

    // send the file
    FILE *fp = fopen(url, "r");

    // char *buffer = (char *)malloc(sizeof(char)*1025);
    char buffer[1025];
    int n;
    int totalBytes = 0;

    while (1)
    {   
        n = fread(buffer, 1, 1000, fp);
        if(n <= 0) break;

        send(new_socket, buffer, n, 0);
        // printf("ABC\n");
        totalBytes += n;
    }

    // printf("xxx\n");
    printf("\nTotal bytes sent: %d\n", totalBytes);
    fclose(fp);
    printf("File sent successfully!\n");
    // free(buffer);
}

// new functions from here
void set_header(Response *res, char *name, char *val){
    Header *h = (Header *)malloc(sizeof(Header));
    h->name = strdup(name);
    h->values = strdup(val);

    h->next = res->headers;
    res->headers = h;
}

void set_header_and_HTTPversion(int status_code, struct Response *response)
{   
    if(strlen(response->status_message) == 0){
        if (status_code == 200)
            strcpy(response->status_message, "OK");
        else if (status_code == 400)
            strcpy(response->status_message, "Bad Request");
        else if (status_code == 403)
            strcpy(response->status_message, "Forbidden");
        else if (status_code == 404)
            strcpy(response->status_message, "Not Found");
        else if (status_code == 302)
            strcpy(response->status_message, "Found");
    }
    

    strcpy(response->HTTP_version, "HTTP/1.1");
    response->status_code = status_code;

    // no default headers if redirect
    if(status_code == 302) return;

    // start setting default headers
    time_t t = time(NULL);
    struct tm tmarst = *localtime(&t);
    // header->values = modifydate(3, tmarst);
    char *ptr = modifydate(3, tmarst);
    char date[100];
    strcpy(date, ptr);
    free(ptr);

    if(cors){
        set_header(response, "Access-Control-Allow-Origin", origin);
        set_header(response, "Access-Control-Allow-Methods", "GET, PUT, POST, DELETE, OPTIONS");
        set_header(response, "Access-Control-Allow-Headers", "Cookie, Authorization");
    }
    set_header(response, "Expires", date);
    set_header(response, "Cache-Control", "no-store always");
    set_header(response, "Content-language", "en-us");
    set_header(response, "Connection", "keep-alive");   
    
}


Response *new_response(){
    Response *temp = (Response *)malloc(sizeof(Response));
    temp->body = NULL;
    temp->headers = NULL;
    temp->status_code = -1;
    temp->status_message[0] = '\0';
    
    return temp;
}

// send the response including the body
// generally used by user when sending custom response
// the headers in this will be set by the user
void send_response(Response *res, int sock){

    int status = res->status_code == -1 ? 200 : res->status_code;
    Header *user_header = res->headers;
    res->headers = NULL;

    // set default headers and HTTP version
    // set some default set of headers on the basis of the status code
    // default headers CORS, content-length, content-type, 
    set_header_and_HTTPversion(status, res);

    // set the content length and text format headers
    int len = res->body ? strlen(res->body) : 0;
    char str[10];
    sprintf(str, "%d", len);

    // set the headers
    set_header(res, "Content-Length", str);
    set_header(res, "Content-Type", "text/*");
    
    // check if the user has already set them, if yes, over ride the default
    // very important step
    int index = 0;
    while(user_header){
        // check if this header exists in default headers
        index = 0;
        Header *h = res->headers;
        Header *temp = user_header->next;
        while(h){
            if(strcmp(user_header->name, h->name) == 0){
                if(h->values) free(h->values);
                h->values = strdup(user_header->values);
                index = 1;

                // free the header
                user_header->next = NULL;
                free_header_request(user_header);
                break;
            }

            h = h->next;
        }
        if(index == 0){
            // means the goven header is not in the default headers
            user_header->next = res->headers;
            res->headers = user_header;
        }

        user_header = temp;;
    }

    // send the headers, including version and status message
    send_response_header(sock, res);
    // send the body if it is not present
    if(res->body) send(sock, res->body, strlen(res->body), 0); 
}

void CORS_enable(char *address){
    cors = 1;
    strcpy(origin, address);
}

void set_status_message(Response *res, char *msg){
    if(strlen(msg) > 100){
        perror("To large status message in set_status_message\n");
        exit(0);
    }
    strcpy(res->status_message, msg);
}

void set_body(Response *res, char *body){
    if(res->body) free(res->body);
    res->body = strdup(body);
}


