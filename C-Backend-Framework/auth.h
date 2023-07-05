#include <time.h>

typedef struct sessionTable{
    struct sessionTable *next;
    char session_token[50];
    time_t timestamp;
    char *user_info;
}session_table;

typedef struct sessionHead{
    session_table *head;
}session_head;

#include "request.h"
int check_authentication(Request *, int);
int user_loader(Request *req, char *pk, int socket, int status);
char *generate_session_token( char *pk);
void LoginManager();
void login_required(char *end);


