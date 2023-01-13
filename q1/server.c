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
int main()
{
    int sockfd, newSockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create a socket :(");
        exit(0);
    }
    struct sockaddr_in serveAddr, clientAddr;
    serveAddr.sin_addr.s_addr = INADDR_ANY;
    serveAddr.sin_family = AF_INET;
    serveAddr.sin_port = htons(2000);
    if (bind(sockfd, (struct sockaddr *)&serveAddr, sizeof(serveAddr)) < 0)

    {
        perror("Unable to bind the socket to local address :(");
        exit(0);
    }
    printf("Server is listening at %d\n", INADDR_ANY);
    if (listen(sockfd, 5) < 0)
    {
        perror("Unable to listen");
        exit(0);
    }
    int cliLen = sizeof(clientAddr);
    char buff[100];
    time_t t;
    while (1)
    {
        if ((newSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, (socklen_t*)&cliLen)) < 0)
        {
            perror("Unable to accept the connection :(");
            exit(0);
        }
        // printf("%d", newSockfd);
        printf("A new client connected\n");
        time(&t);
        strcpy(buff, ctime(&t));
        printf("%s",buff);
        printf("%d ooo",strlen(buff));
        if (send(newSockfd, buff, strlen(buff) + 1, 0) < 0)
        {
            perror("Unable to send the message to client :( error");
        }
        // printf("Sent: %d",x);
        close(newSockfd);
    }

    return 0;
}
