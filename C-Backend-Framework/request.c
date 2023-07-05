#include "common.h"
#include "request.h"
#include "dictionary.h"

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

// frees the request data structures
void free_request(struct Request *request)
{
    if (request)
    {   
        // printf("ABC\n");
        if (request->headers)
            free_header_request(request->headers);
        // printf("ABC\n");
        
        if (request->url)
            free(request->url);
        // printf("ABC\n");
        
        if (request->HTTP_version)
            free(request->HTTP_version);
        // printf("ABC\n");
        
        if (request->enttity_body)
            free(request->enttity_body);
        // printf("ABC\n");
        
        destroy_dictionary(&request->query_params);
        destroy_dictionary(&request->current_user);
        // printf("ABC\n");

        free(request);
        // printf("ABC\n");

    }
}

void extract_query_params(Request *req)
{
    // req->url has been allocated memory in heap
    char *original = req->url;
    char *url_out = strtok(req->url, "?");
    char *query_part = strtok(NULL, "?");

    if (!query_part)
        return; // means there are no query parameters
    else
    {
        req->url = strdup(url_out);

        // converting the parameters to dictionary
        char *data[100];
        int index = 0;
        data[index++] = strtok(query_part, "&");
        char *pt = data[0];
        while (pt)
        {
            pt = strtok(NULL, "&");
            data[index++] = pt;
        }

        // splitting
        index = 0;
        pt = data[index];
        while (pt)
        {
            pt = data[index++];
            if (pt)
            {
                char *key = strtok(pt, "=");
                char *val = strtok(NULL, "=");
                // these key and value do not have memory allocated
                req->query_params.insert(&req->query_params, key, val, 1);
            }
        }

        free(original);
    }
}

// function to extract headers out of the request string
struct Request *parse(char *request)
{
    if (strlen(request) < 3)
        return NULL; // request is too short
    struct Request *requestStruct = malloc(sizeof(struct Request));
    if (!requestStruct)
    {
        return NULL; // if malloc fails, return NULL
    }

    
    memset(requestStruct, 0, sizeof(struct Request)); // initialize the request to 0
    requestStruct->query_params = init_dict();
    requestStruct->current_user = init_dict();


    size_t method_len = strcspn(request, " ");
    if (strncmp(request, "GET", method_len) == 0)
        strcpy(requestStruct->request_method, "GET");
    else if (strncmp(request, "PUT", method_len) == 0)
        strcpy(requestStruct->request_method, "PUT");
    else if (strncmp(request, "POST", method_len) == 0)
        strcpy(requestStruct->request_method, "POST");
    else if (strncmp(request, "DELETE", method_len) == 0)
        strcpy(requestStruct->request_method, "DELETE");
    else
        strcpy(requestStruct->request_method, "UNSUPPORTED");

    // parse url, and version
    request += method_len + 1; // move past method
    size_t url_len = strcspn(request, " ");
    requestStruct->url = malloc(url_len + 1);
    memcpy(requestStruct->url, request, url_len);

    requestStruct->url[url_len] = '\0';
    request += url_len + 1; // move past url
    size_t version_len = strcspn(request, "\r");
    requestStruct->HTTP_version = malloc(version_len + 1);
    memcpy(requestStruct->HTTP_version, request, version_len);
    requestStruct->HTTP_version[version_len] = '\0';
    request += version_len + 2; // move past version

    // extracting the query parameters
    extract_query_params(requestStruct);

    // Parsing the headers
    struct Header *headerStruct = NULL, *lastHeader = NULL;

    while (request[0] != '\r' || request[1] != '\n')
    {
        lastHeader = headerStruct;
        headerStruct = malloc(sizeof(struct Header));
        if (!headerStruct)
        {
            free_request(requestStruct);
            return NULL; // if malloc fails, return NULL
        }
        size_t header_len = strcspn(request, ":");
        headerStruct->name = malloc(header_len + 1);
        if (!headerStruct->name)
        {
            free_request(requestStruct);
            return NULL;
        }
        memcpy(headerStruct->name, request, header_len); // copy the header name
        headerStruct->name[header_len] = '\0';
        request += header_len + 1; // move past header name and ": "

        size_t value_len = strcspn(request, "\r");
        headerStruct->values = malloc(value_len + 1);
        if (!headerStruct->values)
        {
            free_request(requestStruct);
            return NULL;
        }
        memcpy(headerStruct->values, request, value_len); // copy the header value
        headerStruct->values[value_len] = '\0';
        request += value_len + 2; // move past header value and "\r\n"
        headerStruct->next = lastHeader;
    }
    requestStruct->headers = headerStruct;
    // handle blank line before entity body

    return requestStruct;
}

// helper function to receive the data in chunks(50 bytes)
struct Request *receiveHeader(int sockfd)
{

    // char *recvBuffer = (char *)malloc(51);
    char recvBuffer[51];
    char *buff = (char *)malloc(51 * sizeof(char));
    strcpy(buff, "\0");

    struct Request *request;
    int length = 0;
    while (1)
    {
        resetBuffer(recvBuffer, 51);
        int x = recv(sockfd, recvBuffer, 50, 0);
        // printf("%s \n", recvBuffer);
        // printf("x: %d", x);
        if (x <= 0)
        {
            printf("Error receiving data!\n");
            return NULL;
        }

        // we have a problem here
        strcat(buff, recvBuffer);
        char *endOfHeader = strstr(recvBuffer, "\r\n\r\n");

        if (endOfHeader)
        {
            // all headers received, now parsing the headers
            endOfHeader += 3;
            request = parse(buff);
            Header *h = request->headers;

            int content_length = -1;
            while (h)
            {
                if (strcmp(h->name, "Content-Length") == 0)
                {
                    content_length = atoi(h->values);
                    break;
                }

                h = h->next;
            }

            if (content_length == -1 || content_length == 0)
            {
                // skip the content part
                request->enttity_body = NULL;
                break;
            }

            // receive the reaming bytes of the content
            // msg length in current message
            request->enttity_body = (char *)malloc(content_length + 1);
            int msg_received = x - (endOfHeader - recvBuffer + 1);
            int more_to_receive = content_length - msg_received;

            while (more_to_receive)
            {
                resetBuffer(recvBuffer, 51);
                int y = recv(sockfd, recvBuffer, 50, 0);
                buff = realloc(buff, strlen(buff) + 51);
                strcat(buff, recvBuffer);
                more_to_receive -= y;
            }
            char *end = strstr(buff, "\r\n\r\n");
            end += 4;
            memcpy(request->enttity_body, end, content_length);
            request->enttity_body[content_length] = '\0';
            break;
        }

        char *temp = realloc(buff, strlen(buff) + 51);
        if (temp == NULL)
        {
            printf("Error reallocating memory!\n");
            return NULL;
        }
        buff = temp;
    }

    // get the entity body head and free the buff
    free(buff);
    return request;
}

Dictionary get_json(Request *req)
{

    Header *h = req->headers;
    while (h)
    {
        if (strcmp(h->name, "Content-Type") == 0)
            break;
        h = h->next;
    }

    Dictionary d = init_dict();
    if(!h){
        perror("No payload in request\n");
        exit(EXIT_SUCCESS);
    }
    char *test = strstr(h->values, "application/json");
    if (!test)
    {
        perror("Payload is not json\n");
        exit(EXIT_SUCCESS);
    }

    char *body = req->enttity_body;

    // raise error if data type is not json
    // check if json empty
    // if not, start parsing
    while (1)
    {
        // get the first and second " or '

        int start, end;
        int start1 = strcspn(body, "\"");
        int start2 = strcspn(body, "'");
        start = start1 <= start2 ? start1 : start2;
        body += start + 1; // body points to the start of the key

        int end1 = strcspn(body, "\"");
        int end2 = strcspn(body, "'");
        end = end1 <= end2 ? end1 : end2; // end is the length of the key

        // got the key
        char *key = malloc(end + 1);
        char *value;
        memcpy(key, body, end); // copying the contents of the key
        key[end] = '\0';
        // get the :
        int rem = strcspn(body, ":");
        body += rem + 1; // body points to the next of :

        // get the value (how ?) and insert entry in dict
        // check if its string or not
        int isString = 0;
        start1 = strcspn(body, "\"");
        start2 = strcspn(body, "'");
        start = start1 <= start2 ? start1 : start2;

        end1 = strcspn(body, ",");
        end2 = strcspn(body, "}");
        end = end1 <= end2 ? end1 : end2;

        if (start < end)
        {
            // it is a string
            body += start + 1; // body points to the value
            end1 = strcspn(body, "'");
            end2 = strcspn(body, "\"");
            end = end1 <= end2 ? end1 : end2; // end is the length of the
            value = (char *)malloc(end + 1);
            memcpy(value, body, end); // copying the contents of the key
            value[end] = '\0';
            body += end + 1;
            d.insert(&d, key, value, 1);
        }
        else
        {
            // it is not a string
            isString = 0;
            int temp = 1, end = 0;
            char *start_string = body;

            while (1)
            {
                if (*body == ',' || *body == '}' || *body == '\n')
                    break;
                if (*body == ' ' && temp != 0){
                    body++;
                    continue;
                }
                if (*body == ' ' && temp == 0)
                    break;
                if (temp == 1)
                    start_string = body;
                temp = 0;
                end++;
                body++;
            }

            // starting from start_string, copy end characters in value
            value = (char *)malloc(end + 1);
            int val;
            memcpy(value, start_string, end);
            value[end] = '\0';
            // printf("Value is %s\n", value);
            if (isBool(value) || isNull(value))
            {
                d.insert(&d, key, value, 0);
            }
            else
            {
                val = atoi(value);
                d.insert(&d, key, &val, 0);
            }
        }

        free(key);
        free(value);

        // get the ,
        char *point = strstr(body, ":");
        // if we have , continue, else break
        if (!point)
            break;
    }

    return d;
}


Dictionary get_form_data(Request *req)
{
    Header *h = req->headers;
    while (h)
    {
        if (strcmp(h->name, "Content-Type") == 0)
            break;
        h = h->next;
    }

    if(!h){
        perror("No form data in payload\n");
        exit(EXIT_SUCCESS);
    }

    Dictionary d = init_dict();
    char *test = strstr(h->values, "application/x-www-form-urlencoded");
    if (!test)
    {
        perror("Payload is not form data\n");
        exit(EXIT_SUCCESS);
    }

    char *body = req->enttity_body;
    // extracting the form data from here
    // split by &
    char *data[100];
    int index = 0;
    data[index++] = strtok(body, "&");
    char *ptr = data[0];
    while (ptr)
    {
        ptr = strtok(NULL, "&");
        data[index++] = ptr;
    }

    // split by =
    // splitting
    index = 0;
    ptr = data[index];
    while (ptr)
    {
        ptr = data[index++];
        if (ptr)
        {
            char *key = strtok(ptr, "=");
            char *val = strtok(NULL, "=");
            // split by +
            for (int i = 0; i < strlen(key); i++)
            {
                if(key[i] == '+') key[i] = ' ';
            }

            char new_val[strlen(val)+1];
            int index = 0;
            for (int i = 0; i < strlen(val); i++)
            {
                if(val[i] == '+') val[i] = ' ';
                if(val[i] == '%'){
                    // take i+1 and i+2
                    char hex[3];
                    i++;
                    hex[0] = val[i++];
                    hex[1] = val[i];
                    hex[2] = '\0';
                    char ch = strtoul(hex, NULL, 16);
                    val[i] = ch;
                }
                new_val[index++] = val[i];
            }
            new_val[index++] = '\0';

            // these key and value have no memory allocated
            // printf("%s : %s\n", key, new_val);
            d.insert(&d, key, new_val, 1);
        }
    }
    return d;
}

char *get_header(Request *req, char *name){
    Header *h = req->headers;
    while(h){
        if(strcmp(h->name, name) == 0){
            return h->values;
        }
        h = h->next;
    }

    return NULL;
}
