// Server Implementation
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
#include <ctype.h>
// parse the http request
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
typedef struct Request
{
    Method request_method;
    char *ip;  // client ip
    char *url; // request url

    char *HTTP_version;     // HTTP version
    struct Header *headers; // request headers
    char *enttity_body;     // request body
} Request;
typedef struct Response
{
    char *HTTP_version;
    int status_code;
    char *status_message;
    struct Header *headers;
    char *entity_body;
} Response;
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
void free_request(struct Request *request)
{
    if (request)
    {
        if (request->headers)
            free_header_request(request->headers);
        if (request->url)
            free(request->url);
        if (request->HTTP_version)
            free(request->HTTP_version);
        if (request->enttity_body)
            free(request->enttity_body);
        free(request);
    }
}
struct Request *parse(const char *request)
{
    if (strlen(request) < 3)
        return NULL; // request is too short
    struct Request *requestStruct = malloc(sizeof(struct Request));
    if (!requestStruct)
    {
        return NULL; // if malloc fails, return NULL
    }
    memset(requestStruct, 0, sizeof(struct Request)); // initialize the request to 0
    size_t method_len = strcspn(request, " ");
    if (strncmp(request, "GET", method_len) == 0)
        requestStruct->request_method = GET;
    else if (strncmp(request, "PUT", method_len) == 0)
        requestStruct->request_method = PUT;
    else
        requestStruct->request_method = UNSUPPORTED;

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
    size_t len = strcspn(request, "\r");
    if (request[len + 2] == '\r')
        len += 2;
    request += len + 2; // move past "\r\n"

    // parse entity body
    len = strlen(request);
    requestStruct->enttity_body = malloc(len + 1);
    if (!requestStruct->enttity_body)
    {
        free_request(requestStruct);
        return NULL;
    }
    memcpy(requestStruct->enttity_body, request, len);
    requestStruct->enttity_body[len] = '\0';
    return requestStruct;
}
void print_request(struct Request *request)
{
    if (request)
    {
        printf("Method: %d\n", request->request_method);
        printf("Request-URI: %s\n", request->url);
        printf("HTTP-Version: %s\n", request->HTTP_version);
        puts("Headers:");
        struct Header *h;
        for (h = request->headers; h; h = h->next)
        {
            printf("%32s: %s\n", h->name, h->values);
        }

        puts("message-body:");
        puts(request->enttity_body);
    }
}
// helper function to receive the data in chunks(50 bytes)
char *receiveHeader(int sockfd)
{

    char *recvBuffer = (char *)malloc(51 * sizeof(char));
    char *buff = (char *)malloc(51 * sizeof(char));
    strcpy(buff, "\0");
    char *startingEntityBody;
    int length = 0;
    while (1)
    {
        resetBuffer(recvBuffer, 51);
        int x = recv(sockfd, recvBuffer, 50, 0);
        // printf("x: %d", x);
        if (x <= 0)
        {
            printf("Error receiving data!\n");
            return;
        }
        strcat(buff, recvBuffer);
        char *endOfHeader = strstr(recvBuffer, "\r\n\r\n");
        if (endOfHeader)
        {
            startingEntityBody = strdup(recvBuffer);
            break;
        }
        realloc(buff, strlen(buff) + 51);
    }
    // printf("Header received succesfully!\n");
    free(recvBuffer);
    return buff;
}
void receive_and_write_to_file(int sockfd, char *url, int contentLength, char *startingMsg)
{
    printf("Writing to file: %s\n", url);
    printf("Content-Length: %d\n", contentLength);
    printf("Starting message: %s\n", startingMsg);
    FILE *fp = fopen(url, "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(startingMsg, "") != 0)
        fwrite(startingMsg, 1, strlen(startingMsg), fp);
    printf("Starting message: %s\n", startingMsg);

    char *recvBuffer = (char *)malloc(1025 * sizeof(char));
    int length = 0;
    while (1)
    {
        resetBuffer(recvBuffer, 1025);

        int x = recv(sockfd, recvBuffer, 1024, 0);
        printf("x: %d\n", x);
        printf("%s\n", recvBuffer);
        if (x == 0)
        {
            printf("Connection closed!\n");
            break;
        }
        // printf("writing to file: %s\n", recvBuffer);
        fwrite(recvBuffer, 1, x, fp);
        length += x;
        if (length >= contentLength)
            break;
    }
    printf("file downloaded successfully! Size: %d bytes\n", length + strlen(startingMsg));
    fclose(fp);
    free(recvBuffer);
}
// helper function to reset the buffer
void resetBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = '\0';
    }
}
void send_response_header(int client_sockfd, struct Response *response)
{
    char *responseString = malloc(strlen(response->HTTP_version) + 1);
    strcpy(responseString, response->HTTP_version);
    strcat(responseString, " ");
    char *status_code = malloc(4 * sizeof(char));
    sprintf(status_code, "%d", response->status_code);
    malloc(strlen(status_code) + 1);
    strcat(responseString, status_code);
    strcat(responseString, " ");
    malloc(strlen(response->status_message) + 2);
    strcat(responseString, response->status_message);
    strcat(responseString, "\r\n");
    struct Header *h;
    for (h = response->headers; h; h = h->next)
    {
        malloc(strlen(h->name) + 2);
        strcat(responseString, h->name);
        strcat(responseString, ": ");
        malloc(strlen(h->values) + 2);
        strcat(responseString, h->values);
        strcat(responseString, "\r\n");
    }
    strcat(responseString, "\r\n");
    printf("WHATS IN RESPONSE STRING:\n");
    printf("%s\n", responseString);
    send(client_sockfd, responseString, strlen(responseString), 0); // send the header and HTTP version
}
void set_header_and_HTTPversion(int status_code, struct Response *response)
{
    char *status_message = malloc(10 * sizeof(char));
    if (status_code == 200)
        status_message = "OK";
    else if (status_code == 400)
        status_message = "Bad Request";
    else if (status_code == 403)
        status_message = "Forbidden";
    else if (status_code == 404)
        status_message = "Not Found";
    response->HTTP_version = "HTTP/1.1";
    response->status_code = status_code;
    response->status_message = status_message;
    struct Header *header = malloc(sizeof(struct Header));
    response->headers = header;
    header->name = strdup("Expires");
    header->values = strdup("Thu, 01 Dec 1994 16:00:00 GMT");
    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Cache-Control");
    header->values = strdup("no-store always");
    header->next = malloc(sizeof(struct Header));
    header = header->next;
    header->name = strdup("Content-language");
    header->values = strdup("en-us");
    header->next = NULL;
}
void send_put_response(int client_sockfd, int status_code)
{
    char *put_response = malloc(100 * sizeof(char));
    if (status_code == 200)
    {
        strcpy(put_response, "HTTP/1.1 200 OK\r\n");
    }
    else if (status_code == 400)
    {
        strcpy(put_response, "HTTP/1.1 400 Bad Request\r\n");
    }
    else if (status_code == 403)
    {
        strcpy(put_response, "HTTP/1.1 403 Forbidden\r\n");
    }
    else if (status_code == 404)
    {
        strcpy(put_response, "HTTP/1.1 404 Not Found\r\n");
    }
    strcat(put_response, "\r\n");
    send(client_sockfd, put_response, strlen(put_response), 0);
}
void send_response_file(int new_socket, char *url)
{
    // some bug it is not sending the whole file

    // send the file
    FILE *fp = fopen(url, "r");
    char *buffer = malloc(1024 * sizeof(char));
    int n;
    int totalBytes = 0;
    while (1)
    {
        printf("%s\n", buffer);
        if ((n = fread(buffer, 1, 1024, fp)) <= 0)
        {
            break;
        }
        send(new_socket, buffer, n, 0);
        totalBytes += n;
    }
    printf("\nTotal bytes sent: %d\n", totalBytes);
    fclose(fp);
    printf("File sent successfully!\n");
}
int find_content_length_value(struct Request *request)
{
    struct Header *h;
    for (h = request->headers; h; h = h->next)
    {
        char *len = strdup(h->name); // HTTP RFC says that header names are case-insensitive
        for (int i = 0; len[i]; i++)
        {
            len[i] = tolower(len[i]);
        }
        // printf("len: %s\n", len);
        if (strcmp(len, "content-length") == 0)
        {
            // printf("content-length: %s\n", h->values);
            return atoi(h->values);
        }
    }
    return -1;
}
int main()
{

    // normal tcp server routine
    int server_fd, new_socket;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address, client_address;
    server_address.sin_family = AF_INET;
    inet_aton("127.0.0.1", &server_address.sin_addr);
    server_address.sin_port = htons(8081);
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
        printf("Waiting for new connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("New connection accepted from %s:%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        if (!fork())
        {
            // handle the client here
            close(server_fd);
            char *buff;
            // receiveWrapper(new_socket, &buff);
            buff = receiveHeader(new_socket);
            // printf("Received request:\n%s\n", buff);
            // printf("WTF:::000\n");
            // printf("Received request:\n%s\n", buff);
            struct Request *req = parse(buff);
            // printf("wtf:::111\n");
            printf("Received request:\n");
            print_request(req);
            struct Response *response = malloc(sizeof(struct Response));
            if (req->request_method == 2) // Unsupported method
            {
                set_header_and_HTTPversion(400, response);
                send_response_header(new_socket, response);
                // exit(EXIT_SUCCESS);
            }
            else
            {

                // check if the file is a directory
                struct stat path_stat;
                if (!stat(req->url, &path_stat) && S_ISDIR(path_stat.st_mode))
                {
                    // file is a directory then return 403
                    if (req->request_method == 1)
                    {
                        send_put_response(new_socket, 403);
                        exit(EXIT_SUCCESS);
                    }
                    set_header_and_HTTPversion(403, response);
                    send_response_header(new_socket, response);
                    exit(EXIT_SUCCESS);
                }
                if ((req->request_method == 0))
                {
                    // handle GET request
                    // check if the file exists
                    if (access(req->url, F_OK | R_OK) < 0)
                    {
                        // file doesn't exist or can't be read
                        set_header_and_HTTPversion(404, response);
                        send_response_header(new_socket, response);
                        printf("File doesn't exist or can't be read");
                        exit(EXIT_SUCCESS);
                    }

                    // file exists and can be read
                    set_header_and_HTTPversion(200, response);
                    struct Header *h = response->headers;
                    struct stat st;
                    stat(req->url, &st);
                    char *content_length = malloc(100 * sizeof(char));
                    sprintf(content_length, "%ld", st.st_size);
                    h->next = malloc(sizeof(struct Header));
                    h = h->next;
                    h->name = strdup("Content-Length");
                    h->values = strdup(content_length);
                    h->next = malloc(sizeof(struct Header));
                    h = h->next;
                    h->name = strdup("Last-Modified");
                    // h->values = strdup(ctime(&st.st_mtime));
                    h->values = strdup("Mon, 26 Mar 2018 20:54:02 GMT");
                    h->next = NULL;
                    // check if the file is a pdf
                    char *extension = strrchr(req->url, '.');
                    printf("%s\n", extension);
                    h->next = malloc(sizeof(struct Header));
                    if (h->next == NULL)
                    {
                        printf("malloc failed\n");
                    }
                    h = h->next;
                    h->name = strdup("Content-Type");

                    if (strcmp(extension, ".pdf") == 0)
                    {
                        // file is a pdf
                        h->values = strdup("application/pdf");
                    }
                    else if (strcmp(extension, ".html") == 0)
                    {
                        // file is a html
                        h->values = strdup("text/html");
                    }
                    else if (strcmp(extension, ".jpg") == 0)
                    {
                        // file is a txt
                        h->values = strdup("image/jpeg");
                    }
                    else
                    {
                        // any other file
                        h->values = strdup("text/*");
                    }
                    h->next = NULL;
                    send_response_header(new_socket, response);
                    send_response_file(new_socket, req->url);
                }
                else
                {
                    // handle PUT request
                    int content_length = find_content_length_value(req) - strlen(req->enttity_body);
                    printf("content length: %d\n", content_length);
                    if (content_length == -1)
                    {
                        // content length not found
                        send_put_response(new_socket, 400);
                        exit(EXIT_SUCCESS);
                    }
                    receive_and_write_to_file(new_socket, req->url, content_length, req->enttity_body);
                    send_put_response(new_socket, 200);
                }
            }
            printf("Connection closed with client\n");
            exit(EXIT_SUCCESS);
        }
        close(new_socket);
    }
    // free_request(req);
    return 0;
}
