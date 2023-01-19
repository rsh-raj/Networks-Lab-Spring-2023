#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#define _GNU_SOURCE
#include <signal.h>
#include <poll.h>
int main()
{
    int sockfd, clientPort = 20001;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable tot create a socket :(");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in clientAddr, serverAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(clientPort);
    inet_aton("127.0.0.1", &clientAddr.sin_addr);
    socklen_t sockLen = sizeof(clientAddr);
    if (bind(sockfd, (const struct sockaddr *)&clientAddr, sockLen) < 0)
    {
        perror("Binding failed :(");
    }
    char buff[100];
    for (int i = 0; i < 100; i++)
    {
        buff[i] = '\0';
    }
    struct pollfd pollInfo;
    pollInfo.fd = sockfd;
    pollInfo.events = POLLIN;
    printf("waiting for response from server\n");
    int retryCount=0;
    while (1)
    {
        if(retryCount==5){
            printf("Timeout exceeded!!");
            break;
        }
        int returnVal = poll(&pollInfo, 1, 3000);
        if (returnVal < 0)
        {
            perror("poll failed :( ");
            exit(EXIT_FAILURE);
        }
        else if (returnVal == 0)
        {
            printf("Unable to get response from server, Retrying...\n");
            retryCount++;
        }
        else
        {
            recvfrom(sockfd, buff, 100, 0, (struct sockaddr *)&serverAddr, &sockLen);
            printf("%s\n", buff);
            break;
        }
    }
    close(sockfd);

    return 0;
}