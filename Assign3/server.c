#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
void resetBuffer(char *buff, int size);
void receiveWrapper(int sockfd, char **buff);
int main(int argc, char **argv)
{
    srand(time(NULL));
    if (argc <= 1)
    {
        printf("Usage: ./server1 <port> or .server2 <port>\n");
        exit(0);
    }
    int port = atoi(argv[1]);
    int sockfd, newSockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create a socket :(");
        exit(0);
    }
    struct sockaddr_in serveAddr, clientAddr;
    inet_aton("127.0.0.1", &serveAddr.sin_addr);
    serveAddr.sin_family = AF_INET;
    serveAddr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&serveAddr, sizeof(serveAddr)) < 0)

    {
        perror("Unable to bind the socket to local address :(");
        exit(0);
    }
    printf("Server is listening at %d\n", port);
    if (listen(sockfd, 5) < 0)
    {
        perror("Unable to listen\n");
        exit(0);
    }
    int cliLen = sizeof(clientAddr);
    char *buff = (char *)malloc(100 * sizeof(char));
    time_t t;
    while (1)
    {
        int load = rand() % 100 + 1;

        if ((newSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, (socklen_t *)&cliLen)) < 0)
        {
            perror("Unable to accept the connection :(\n");
            exit(0);
        }
        printf("A new client connected\n");

        resetBuffer(buff, 100);
        receiveWrapper(newSockfd, &buff);
        printf("Received from client: %s \n", buff);
        if (strcmp(buff, "Send load") == 0)
        {
            // send load
            resetBuffer(buff, 100);
            sprintf(buff, "%d", load);
            if (send(newSockfd, buff, strlen(buff) + 1, 0) < 0)
            {
                perror("Unable to send the message to client :( error");
            }
            printf("Load: %d sent to client and client disconnected\n",load);
        }

        else if (strcmp(buff, "Send Time"))
        {
            time(&t);

            strcpy(buff, ctime(&t));

            if (send(newSockfd, buff, strlen(buff) + 1, 0) < 0)
            {
                perror("Unable to send the message to client :( error");
            }
            printf("Date and time sent to client and client disconnected\n");
        }
        else
        {
            printf("Invalid message from client\n");
        }
        close(newSockfd);
    }
    free(buff);

    return 0;
}

void resetBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = '\0';
    }
}
// helper function to receive the data in chunks(50 bytes)
void receiveWrapper(int sockfd, char **buff)
{
    char recvBuffer[50];
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
    }
}
