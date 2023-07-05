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

typedef struct Header
{
    char *name;
    char *values; // if there are multiple values, they are separated by a comma
    struct Header *next;

} Header;

typedef struct node{
    char *key;
    void *val;
    int isString;
    struct node *next;
}node;

typedef struct dict{
    node *head;
    void (*insert)(struct dict *dictionary, char *key, void *value, int isString);
    // Search looks for a given key in the dictionary and returns its value if found or NULL if not.
    void * (*search)(struct dict *dictionary, char *key);
}Dictionary;

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

typedef struct Response
{
    char HTTP_version[20];
    int status_code;
    char status_message[100];
    struct Header *headers;
    char *body;
} Response;

#define IN Request *, int
#define OUT char *

// void destroy_dictionary(Dictionary *dict);
void insert_dict(Dictionary *dictionary, char *key, void *value, int isString);
void *search_dict(Dictionary *dictionary, char *key);
Dictionary init_dict();
void print_dict(Dictionary d);
Dictionary get_json(Request *);
Dictionary get_form_data(Request *);
// void send_response_header(int client_sockfd, struct Response *response);
// void send_response_file(int new_socket, char *url);
// void set_header_and_HTTPversion(int status_code, struct Response *response);
char *render_template(int socket, char *path);
char *jsonify(int socket, int status, Dictionary *d, int isList, int size);
void redirect(int new_socket, char *end, char *pk);
void flash(int new_socket, char *msg);
char *get_header(Request *req, char *name);

// int user_loader(Request *req, char *pk, int socket);
void LoginManager();
void login_required(char *end);
void set_header(Response *res, char *name, char *val);
void CORS_enable(char *address);
void send_response(Response *res, int sock);
void set_status_message(Response *res, char *msg);
void set_body(Response *res, char *body);
Response *new_response();
void create_app(int port);
void add_route(char *r, OUT (*fn_ptr)(IN), char **methods, int num_methods);