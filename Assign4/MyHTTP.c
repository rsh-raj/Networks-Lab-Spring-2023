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
void receiveWrapper(int sockfd, char **buff)
{
    char recvBuffer[50];
    *buff = (char *)malloc(51 * sizeof(char));
    while (1)
    {
        resetBuffer(recvBuffer, 50);
        int x = recv(sockfd, recvBuffer, 50, 0);
        if (recvBuffer[x - 1] != '\0')
        {
            recvBuffer[x] = '\0';
            strcat(*buff, recvBuffer);
        }
        else
        {

            strcat(*buff, recvBuffer);
            break;
        }
        *buff = realloc(*buff, strlen(*buff) + 51);
    }
}
// helper function to reset the buffer
void resetBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = '\0';
    }
}
void send_response(int client_sockfd, struct Response *response)
{
    char *responseString = malloc(strlen(response->HTTP_version) + 1);
    strncat(responseString, response->HTTP_version, strlen(response->HTTP_version));
    strncat(responseString, " ", 1);
    char *status_code = malloc(4 * sizeof(char));
    sprintf(status_code, "%d", response->status_code);
    malloc(strlen(status_code) + 1);
    strncat(responseString, status_code, strlen(status_code));
    strncat(responseString, " ", 1);
    malloc(strlen(response->status_message) + 2);
    strncat(responseString, response->status_message, strlen(response->status_message));
    strncat(responseString, "\r\n", 2);
    struct Header *h;
    for (h = response->headers; h; h = h->next)
    {
        malloc(strlen(h->name) + 2);
        strncat(responseString, h->name, strlen(h->name));
        strncat(responseString, ": ", 2);
        malloc(strlen(h->values) + 2);
        strncat(responseString, h->values, strlen(h->values));
        strncat(responseString, "\r\n", 2);
    }
    strncat(responseString, "\r\n", 2);
    send(client_sockfd, responseString, strlen(responseString), 0); // send the header and HTTP version
}
struct Header *set_header_and_HTTPversion(int status_code, struct Response *response)
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
    header->name = "Expires";
    header->values = "Thu, 01 Dec 1994 16:00:00 GMT";
    header->name = "Cache-Control";
    header->values = "no-store always";
    header->next = NULL;
    response->headers = header;
    return header;
}
int main()
{
    char *raw_request = "PUT / HTTP/1.1\r\n"
                        "Host: localhost:8080\r\n"
                        "Connection: keep-alive\r\n"
                        "Upgrade-Insecure-Requests: 1\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                        "Accept: application/pdf\r\n"
                        "Date: Mon, 26 Mar 2018 20:54:02 GMT\r\n"
                        "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_3) AppleWebKit/604.5.6 (KHTML, like Gecko) Version/11.0.3 Safari/604.5.6\r\n"
                        "Accept-Language: en-us,en;q=0.8\r\n"
                        "DNT: 1\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "\r\n\r\n"
                        "Usually GET requests don\'t have a body\r\n"
                        "But I don\'t care in this case :)";

    struct Request *req = parse(raw_request);
    print_request(req);

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
    server_address.sin_port = htons(8080);
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
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if (!fork())
        {
            // handle the client here
            close(server_fd);
            char *buff;
            // receiveWrapper(new_socket, &buff);
            struct Request *req = parse(buff);
            print_request(req);
            struct Response *response = malloc(sizeof(struct Response));

            if (req->request_method == 2) // Unsupported method
            {
                set_header_and_HTTPversion(400, response);
                send_response(new_socket, response);
            }
            else if (req->request_method == 0)
            {
                // handle GET request
                // check if the file exists
                if (access(req->url, F_OK | R_OK) < 0)
                {
                    // file doesn't exist or can't be read
                    set_header_and_HTTPversion(404, response);
                    send_response(new_socket, response);
                }
                else
                {
                    // file exists and can be read
                    // check if the file is a directory
                    struct stat path_stat;

                    if (stat(req->url, &path_stat) && S_ISDIR(path_stat.st_mode))
                    {
                        // file is a directory then return 403
                        set_header_and_HTTPversion(403, response);
                        send_response(new_socket, response);
                    }
                    else
                    {
                        // file is not a directory
                        // check if the file is a pdf
                        char *extension = strrchr(req->url, '.');
                        if (strcmp(extension, ".pdf") == 0)
                        {
                            // file is a pdf
                            struct Header *h = set_header_and_HTTPversion(200, response);
                            h->next = malloc(sizeof(struct Header));
                            h->next->name = "Content-Type";
                            h->next->values = "application/pdf";
                            h->next->next = NULL;
                            send_response(new_socket, response);
                            // send the file
                            FILE *fp = fopen(req->url, "r");
                            char *buffer = malloc(1024 * sizeof(char));
                            int n;
                            while ((n = fread(buffer, 1, 1024, fp)) > 0)
                            {
                                send(new_socket, buffer, n, 0);
                            }
                            fclose(fp);
                        }
                        else
                        {
                            // file is not a pdf
                            set_header_and_HTTPversion(403, response);
                            send_response(new_socket, response);
                        }
                    }
                }
            }
            else
            {
                // handle PUT request
            }

            // closing the connection with client and exiting the process
            close(new_socket);
            exit(EXIT_SUCCESS);
        }

        close(new_socket);
    }

    free_request(req);
    return 0;
}
