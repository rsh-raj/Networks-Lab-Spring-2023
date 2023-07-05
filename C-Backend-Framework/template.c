#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include "template.h"
#include "dictionary.h"
#include "common.h"
#include "auth.h"
#include "request.h"
#include "response.h"

extern int auth;

void *render_template(int new_socket, char *p){
    struct stat st;
    char path[100];
    strcpy(path, "templates/");
    strcat(path, p);

    int status_code = 200;
    int x = stat(path, &st);

    struct Response *response = new_response();

    if (x == -1)
    {
        perror("Template does not exist\n");
        exit(EXIT_SUCCESS);
    }

    set_header_and_HTTPversion(status_code, response);

    char content_length[100];
    sprintf(content_length, "%lld", st.st_size);

    set_header(response, "Content-Length", content_length);
    set_header(response, "Content-Type", "text/html");
    send_response_header(new_socket, response);
    free_response(response);
    send_response_file(new_socket, path);

    return NULL;
}


void *jsonify(int new_socket,int status_code, Dictionary *d, int isList, int size){
    char *sendString;
    sendString = convert_dict_to_string(d,isList,size);

    struct Response *response = new_response();
    set_header_and_HTTPversion(status_code, response);

    char content_length[100];
    sprintf(content_length, "%ld", strlen(sendString));

    set_header(response, "Content-Length", content_length);
    set_header(response, "Content-Type", "application/json");

    // printf("XX\n");

    send_response_header(new_socket, response);
    // printf("XX\n");

    free_response(response);
    // printf("XX\n");

    send(new_socket, sendString, strlen(sendString), 0);
    printf("Total bytes sent: %d\n",(int)strlen(sendString));
    printf("Data sent successfully!\n");

    free(sendString);

    return NULL;
}

void redirect(int new_socket, char *end, char *pk){

    struct Response *response = new_response();
    set_header_and_HTTPversion(302, response);

    // printf("endpoint is :%s\n", end);
    set_header(response, "Location", end);
    set_header(response, "Connection", "keep-alive");
    // h->name = strdup("Location");
    // h->values = strdup(end);
    // h->next = malloc(sizeof(Header));
    // h = h->next;
    // h->name = strdup("Connection");
    // h->values = strdup("keep-alive");

    // printf("endpoint is :%s\n", end);


    if(pk && auth){
        char *token = generate_session_token(pk);
        // h->next = malloc(sizeof(Header));
        // h = h->next;
        // h->name = strdup("Set-Cookie");
        char content[300];
        strcpy(content,"session_token=");
        strcat(content, token);
        strcat(content, "; Expires=Sat, 30 May 2024 12:00:00 GMT; Path=/; Domain=127.0.0.1; Secure; HttpOnly");
        // h->values = strdup(content);
        set_header(response, "Set-Cookie", content);
        // h->values = strdup("Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c");
    }

    // printf("endpoint is :%s\n", end)
    send_response_header(new_socket, response);
    free_response(response);
    // close(new_socket);
    // pthread_exit(0);
}

void flash(int new_socket, char *msg){
    char response[1000] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 70\r\n\r\n<html><body></body></html>";
    char popup_code[1000] = "<script>alert('Incorrect Credentials');</script>";
    strcat(response, popup_code);

    send(new_socket, response, strlen(response), 0);
}

