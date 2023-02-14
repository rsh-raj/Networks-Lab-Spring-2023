// Client side
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
void free_response(struct Response *request)
{
    if (request)
    {
        if (request->headers)
            free_header_request(request->headers);
        if (request->status_message)
            free(request->status_message);
        if (request->HTTP_version)
            free(request->HTTP_version);
        free(request);
    }
}
struct Response *parse_response(const char *request)
{
    if (strlen(request) < 3)
        return NULL; // request is too short
    struct Response *responseStruct = malloc(sizeof(struct Request));
    if (!responseStruct)
    {
        return NULL; // if malloc fails, return NULL
    }
    memset(responseStruct, 0, sizeof(struct Request)); // initialize the request to 0

    size_t version_len = strcspn(request, " ");
    responseStruct->HTTP_version = malloc(version_len + 1);
    memcpy(responseStruct->HTTP_version, request, version_len);
    responseStruct->HTTP_version[version_len] = '\0';
    request += version_len + 1; // move past version

    size_t status_code_len = strcspn(request, " ");
    char *status_code = malloc(status_code_len + 1);
    memcpy(status_code, request, status_code_len);
    status_code[status_code_len] = '\0';
    responseStruct->status_code = atoi(status_code);
    request += status_code_len + 1; // move past status code

    size_t status_message_len = strcspn(request, "\r");
    responseStruct->status_message = malloc(status_message_len + 1);
    memcpy(responseStruct->status_message, request, status_message_len);
    responseStruct->status_message[status_message_len] = '\0';
    request += status_message_len + 2; // move past status message and "\r\n"

    // Parsing the headers
    struct Header *headerStruct = NULL, *lastHeader = NULL;

    while (request[0] != '\r' || request[1] != '\n')
    {
        lastHeader = headerStruct;
        headerStruct = malloc(sizeof(struct Header));
        if (!headerStruct)
        {
            free_response(responseStruct);
            return NULL; // if malloc fails, return NULL
        }
        size_t header_len = strcspn(request, ":");
        headerStruct->name = malloc(header_len + 1);
        if (!headerStruct->name)
        {
            free_response(responseStruct);
            return NULL;
        }
        memcpy(headerStruct->name, request, header_len); // copy the header name
        headerStruct->name[header_len] = '\0';
        request += header_len + 1; // move past header name and ": "

        size_t value_len = strcspn(request, "\r");
        headerStruct->values = malloc(value_len + 1);
        if (!headerStruct->values)
        {
            free_response(responseStruct);
            return NULL;
        }
        memcpy(headerStruct->values, request, value_len); // copy the header value
        headerStruct->values[value_len] = '\0';
        request += value_len + 2; // move past header value and "\r\n"
        headerStruct->next = lastHeader;
    }
    responseStruct->headers = headerStruct;
    // handle blank line before entity body
    size_t len = strcspn(request, "\r");
    if (request[len + 2] == '\r')
        len += 2;
    request += len + 2; // move past "\r\n"

    // parse entity body not needed as we are handling it separately
    len = strlen(request);
    responseStruct->entity_body = malloc(len + 1);
    if (!responseStruct->entity_body)
    {
        free_response(responseStruct);
        return NULL;
    }
    memcpy(responseStruct->entity_body, request, len);
    responseStruct->entity_body[len] = '\0';
    return responseStruct;
}
void print_response(struct Response *request)
{
    if (request)
    {
        // printf("Method: %d\n", request->request_method);
        // printf("Request-URI: %s\n", request->url);
        printf("%32sHTTP-Version: %s\n", "", request->HTTP_version);
        printf("%32sStatus-Code: %d\n", "", request->status_code);
        printf("%32sStatus-Message: %s\n", "", request->status_message);

        puts("Headers:");
        struct Header *h;
        for (h = request->headers; h; h = h->next)
        {
            printf("%32s: %s\n", h->name, h->values);
        }

        puts("message-body:");
        puts(request->entity_body);
    }
}
// helper function to receive the data in chunks(1024 bytes)
char *receiveHeader(int sockfd)
{
    char *startingEntityBody = NULL;
    char *recvBuffer = (char *)malloc(1025 * sizeof(char));
    char *buff = (char *)malloc(1025 * sizeof(char));
    strcpy(buff, "\0");
    while (1)
    {
        reset_buff(recvBuffer, 1025);
        int x = recv(sockfd, recvBuffer, 1024, 0);
        if (x <= 0)
        {
            printf("Error receiving data!\n");
            return NULL;
        }
        strcat(buff, recvBuffer);
        char *endOfHeader = strstr(recvBuffer, "\r\n\r\n");
        if (endOfHeader)
        {
            startingEntityBody = strdup(recvBuffer);
            break;
        }
        char *tmp=realloc(buff, strlen(buff) + 1025);
        if(tmp==NULL){
            perror("realloc failed:214\n");
            return NULL;
        }
        buff=tmp;

    }
    printf("Header received succesfully!\n");
    // printf("Header: %s\n", buff);
    free(recvBuffer);
    return buff;
}
void receive_and_write_to_file(int sockfd, char *url, int contentLength, char *startingMsg)
{
    // printf("Writing to file: %s\n", url);
    // printf("Content-Length: %d\n", contentLength);
    // printf("Starting message: %s\n", startingMsg);
    FILE *fp = fopen(url, "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(startingMsg, "") != 0)
        fwrite(startingMsg, 1, strlen(startingMsg), fp);
    char *recvBuffer = (char *)malloc(1025 * sizeof(char));
    int length = 0;
    while (1)
    {
        reset_buff(recvBuffer, 1025);

        int x = recv(sockfd, recvBuffer, 1024, 0);
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
    printf("file downloaded successfully! Size: %ld bytes\n", length + strlen(startingMsg));
    fclose(fp);
    free(recvBuffer);
}
// helper function to reset the buffer
void reset_buff(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = '\0';
    }
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
int find_content_length_value(struct Response *response)
{
    struct Header *h;
    for (h = response->headers; h; h = h->next)
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
char **tokenize_command(char *cmd)
{
    // what the fuck is this malloc errror on:
    // GET http://127.0.0.1/home/rsh-raj/Documents/KGP/sem6/networks/Networks-Lab-Spring-2023/Assign1/Assgn-1.pdf:3000
    int index = 0;
    char temp[5000];

    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(1000 * sizeof(char));

    int cnt = 0;
    int flag = 0;
    int space = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        // remove the starting spaces
        if (flag == 0 && cmd[i] == ' ')
            continue;
        flag = 1;

        cnt = 0;
        if (space == 1 && cmd[i] == ' ')
            continue;
        else if (cmd[i] == ' ')
        {
            temp[cnt++] = cmd[i];
            space = 1;
            continue;
        }

        // index for populating the array
        while (!(cmd[i] == ' ' && cmd[i - 1] != '\\'))
        {
            if (cmd[i] == '\0')
                break;
            if (cmd[i] == '\\')
            {
                i++;
                // skipping the back slash
                temp[cnt++] = cmd[i++];
                continue;
            }
            temp[cnt++] = cmd[i++];
            // added random
        }

        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        char**tmp = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        if (tmp == NULL)
        {
            printf("realloc failed:367\n");
            exit(1);
        }
        cmdarr = tmp;
        cmdarr[index] = (char *)malloc(1000 * sizeof(char));

        if (cmd[i] == '\0')
            break;
    }

    cmdarr[index] = NULL;
    return cmdarr;
}

void getIPandPort(char **tokens, char *IP, int *portnum)
{
    printf("this motherfucker: \n");
    // GET http://127.0.0.1/home/rsh-raj/test.mkv:3000
    int flag = 0;
    int index = 0;
    int cnt = 0;
    char port[10];
    *portnum = 80; // default port number

    for (int i = 0; tokens[1][i] != '\0'; i++)
    {
        printf("this motherfucker2: \n");

        if (tokens[1][i] == '/' && flag == 1)
        {
            // IP begins
            i++;
            while (tokens[1][i] != '/')
            {
                IP[index++] = tokens[1][i++];
            }
            IP[index++] = '\0';
        }

        else if (tokens[1][i] == '/')
            flag = 1;

        else if (tokens[1][i] == ':' && tokens[1][i + 1] != '/')
        {
            flag = 0;
            // port number begins
            i++;
            while (tokens[1][i] != '\0')
            {
                port[cnt++] = tokens[1][i++];
            }
            port[cnt++] = '\0';
            *portnum = atoi(port);
        }

        else
            flag = 0;
    }
}

void send_request_header(int sockfd, char *url, char *host)
{

    printf("Sending request header:\n");
    char *request = (char *)malloc(5000 * sizeof(char));
    if (request == NULL)
    {
        printf("header send malloc failed\n");
        return;
    }
    strcpy(request, "GET ");
    strcat(request, url);
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "Host: ");
    strcat(request, host);
    strcat(request, "\r\n");
    strcat(request, "Connection: close\r\n");
    strcat(request, "Date: ");
    strcat(request, "Thu, 01 Jan 1970 00:00:00 GMT\r\n");
    char *extension = strrchr(url, '.');
    if (!extension)
        strcat(request, "Accept: text/*");
    else if (strcmp(extension, ".html") == 0)
        strcat(request, "Accept: text/html");
    else if (strcmp(extension, ".jpg") == 0)
        strcat(request, "Accept: image/jpeg");
    else if (strcmp(extension, ".pdf") == 0)
        strcat(request, "Accept: application/pdf");
    else
        strcat(request, "Accept: text/*");
    strcat(request, "Accept-Language: en-US,en;q=0.5\r\n");
    strcat(request, "If-Modified-Since: Thu, 01 Jan 1970 00:00:00 GMT\r\n");
    strcat(request, "\r\n");
    printf("Request sent to server is %s and length is %ld bytes \n", request, strlen(request));
    send(sockfd, request, strlen(request), 0); // get request sent to server
}
void send_put_request(int sockfd, char *finalUrl, char *IPaddress, char *contentlength)
{
    char *request = (char *)malloc(5000 * sizeof(char));
    if (request == NULL)
    {
        printf("header send malloc failed\n");
        return;
    }
    strcpy(request, "PUT ");
    strcat(request, finalUrl);
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "Host: ");
    strcat(request, IPaddress);
    strcat(request, "\r\n");
    strcat(request, "Connection: close\r\n");
    strcat(request, "Date: ");
    strcat(request, "Thu, 01 Jan 1970 00:00:00 GMT\r\n");
    strcat(request, "Content-Length: ");
    strcat(request, contentlength);
    strcat(request, "\r\n");
    char *extension = strrchr(finalUrl, '.');
    if (!extension)
        strcat(request, "Content-type: text/*");
    else if (strcmp(extension, ".html") == 0)
        strcat(request, "Content-type: text/html");
    else if (strcmp(extension, ".jpg") == 0)
        strcat(request, "Content-type: image/jpeg");
    else if (strcmp(extension, ".pdf") == 0)
        strcat(request, "Content-type: application/pdf");
    else
        strcat(request, "Content-type: text/*");
    strcat(request, "Content-Language: en-US\r\n");
    strcat(request, "\r\n");
    printf("Request sent to server is %s\n and length is %ld bytes \n", request, strlen(request));
    send(sockfd, request, strlen(request), 0); // get request sent to server
}

int main()
{
    int sockfd;

    int port;
    struct sockaddr_in serv_addr;
    char cmd[1000];
    char IPaddress[20];
   

    while (1)
    {
        reset_buff(IPaddress, 20);
        reset_buff(cmd, 1000);
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Unable to create socket\n");
            exit(0);
        }
        // initiate the command prompt and take the command
        printf("MyOwnBrowser > ");
        fgets(cmd, 1000, stdin);
        cmd[strlen(cmd) - 1] = '\0';
        printf("Command entered : %s\n", cmd);

        // tokenize the command and form an array of strings
        printf("Tokens are : \n");
        char **tokens = tokenize_command(cmd);
        int i = 0;
        while (tokens[i])
        {
            printf("%s\n", tokens[i]);
            i++;
        }

        if (!strcmp(tokens[0], "QUIT"))
            break;

        // parse the tokenized array to get the server IP and port
        getIPandPort(tokens, IPaddress, &port);
        printf("Port is : %d and IP is %s\n", port, IPaddress);

        // sockfd = establish_connection(port, IPaddress);
        serv_addr.sin_family = AF_INET;
        inet_aton(IPaddress, &serv_addr.sin_addr);
        serv_addr.sin_port = htons(port);
        if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
        {
            perror("Unable to connect to server\n");
            continue;
        }
        char *url = tokens[1];
        int count = 0;
        for (i = 0; i < strlen(url); i++)
        {
            if (url[i] == '/')
                count++;
            if (count == 3)
                break;
        }
        url += i;
        size_t len = strcspn(url, ":");
        char *finalUrl = malloc((len + 1) * sizeof(char));
        memcpy(finalUrl, url, len);
        finalUrl[len] = '\0';
        printf("Final url is %s", finalUrl);
        printf("Connection established\n");
        if (strcmp(tokens[0], "GET") == 0)
        {
            send_request_header(sockfd, finalUrl, IPaddress);
            char *response = receiveHeader(sockfd);
            if (response == NULL)
            {
                continue;
            }
            struct Response *responseStruct = parse_response(response);
            print_response(responseStruct);
            char *file_name = strrchr(url, '/');
            file_name++;
            size_t len = strcspn(file_name, ":");
            file_name[len] = '\0';
            printf("File name is %s\n", file_name);
            // char *file_name = "index.txt";
            int file_size = find_content_length_value(responseStruct);
            if (file_size < 0)
            {
                printf("Error content-length header missing\n");
                continue;
            }
            receive_and_write_to_file(sockfd, file_name, file_size - strlen(responseStruct->entity_body), responseStruct->entity_body);
            // //fork a children and display the file
            // int pid = fork();
            // if (pid == 0)
            // {
            //     // strcat(file_name, " &");
            //     char *args[] = {"xdg-open", file_name,NULL};
            //     char *args2[]={"ls","-l",NULL};
            //     // execvp("xdg-open", args);
            //     exit(EXIT_SUCCESS);
            // }
        }
        else if (!strcmp(tokens[0], "PUT"))
        {
            // send the put request
            strcat(finalUrl, "/");       // append a / to the url
            strcat(finalUrl, tokens[2]); // append the file name to the url
            struct stat st;
            if (stat(tokens[2], &st) < 0)
            {
                perror("Error in file opening\n");
                continue;
            }
            char *content_length = malloc(100 * sizeof(char));
            sprintf(content_length, "%ld", st.st_size);
            printf("Content length is %s\n", content_length);
            send_put_request(sockfd, finalUrl, IPaddress, content_length);
            send_response_file(sockfd, tokens[2]);
            char *response = receiveHeader(sockfd);
            if (response == NULL)
            {
                continue;
            }
            struct Response *responseStruct = parse_response(response);
            print_response(responseStruct);
        }
    }
}