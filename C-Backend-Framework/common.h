#ifndef COMMON_H
#define COMMON_H

typedef enum Method
{
    GET,
    PUT,
    UNSUPPORTED
} Method;

typedef struct Header
{
    char *name;
    char *values; // if there are multiple values, they are separated by a comma
    struct Header *next;

} Header;

#include "request.h"
#include "response.h"

#define IN Request *, int
#define OUT char *

typedef struct route
{
    char endpoint[100];
    OUT (*fn_ptr)(IN);
    char *methods[10];
    int num_methods;
    int login_required;
    struct route *next;
} route;


void resetBuffer(char *buff, int size);
int hash(char *s);
void init_hash_table();

#endif /* GRANDPARENT_H */
