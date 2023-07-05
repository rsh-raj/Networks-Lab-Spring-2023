#ifndef REQUEST_H
#define REQUEST_H
#include "dictionary.h"

typedef struct Request
{
    char request_method[20];
    char *ip;  // client ip
    char *url; // request url
    Dictionary query_params;
    Dictionary current_user;
    char *HTTP_version;     // HTTP version
    struct Header *headers; // request headers
    char *enttity_body;     // request body
} Request;

#include "common.h"

void free_header_request(Header *h);
void free_request(struct Request *request);
struct Request *parse(char *request);
struct Request *receiveHeader(int sockfd);
Dictionary get_json(Request *);
Dictionary get_form_data(Request *);
char *get_header(Request *req, char *name);

#endif