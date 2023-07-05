
#ifndef TEMPLATE_H
#define TEMPLATE_H
#include "dictionary.h"
void *render_template(int socket, char *path);
void *jsonify(int socket, int status, Dictionary *d, int isList, int size);
void redirect(int new_socket, char *end, char *pk);
void flash(int new_socket, char *msg);

#endif