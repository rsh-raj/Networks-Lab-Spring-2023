#ifndef DICTIONARY
#define DICTIONARY

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

void destroy_dictionary(Dictionary *dict);
void insert_dict(Dictionary *dictionary, char *key, void *value, int isString);
void *search_dict(Dictionary *dictionary, char *key);
Dictionary init_dict();
void free_dict_node(node *h);
void print_dict(Dictionary d);
char *convert_dict_to_string(Dictionary *d, int isList, int size);
char *toString_dict(node *head, int isList);
int isBool(char *s);
int isNull(char *s);

#endif
