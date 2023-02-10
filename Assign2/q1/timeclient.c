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
int main(int argc,char**argv)
{
    // Create a socket
    int sockfd, serverPort = 20000;
    if (argc > 1)
        serverPort = atoi(argv[1]);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable tot create a socket :(");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in  serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_aton("127.0.0.1", &serverAddr.sin_addr);
    socklen_t sockLen = sizeof(serverAddr);
    char *message="PING: ";
    char buff[100];
    for (int i = 0; i < 100; i++)
    {
        buff[i] = '\0';
    }
    // Polling definition
    struct pollfd pollInfo;
    pollInfo.fd = sockfd;
    pollInfo.events = POLLIN;
    printf("waiting for response from server\n");
    int retryCount=0;
    while (1)
    {
        // Send the ping message to the server
        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&serverAddr, sockLen);
        // If timeout exceeded, exit
        if(retryCount==5){
            printf("Timeout exceeded!!");
            break;
        }
        // Poll the socket for response
        int returnVal = poll(&pollInfo, 1, 3000);
        if (returnVal < 0)
        {
            perror("poll failed :( ");
            exit(EXIT_FAILURE);
        }
        // If no response, retry
        else if (returnVal == 0)
        {
            printf("Unable to get response from server, Retrying...\n");
            retryCount++;
        }
        // If response, print the response
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