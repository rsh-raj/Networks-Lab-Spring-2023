#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
int main(int argc, char **argv)
{
    int sockFd, port = 20000;
    if (argc > 1)
        port = atoi(argv[1]);
    // Create a socket
    if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("unable to create a socket :(");
        exit(1);
    }
    // Bind the socket to the port
    struct sockaddr_in serverAddr, clientAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_aton("127.0.0.1", &serverAddr.sin_addr);
    socklen_t sockLen = sizeof(serverAddr), clilen;
    if (bind(sockFd, (const struct sockaddr *)&serverAddr, sockLen) < 0)
    {
        perror("Unable to bind the socket to given address :(");
        exit(1);
    }
    printf("Server is running at port:%d\n", port);
    char buff[100];
    while (1)
    {
        // Receive the message from the client and send the current time
        recvfrom(sockFd, buff, 100, 0, (struct sockaddr *)&clientAddr, &clilen);
        time_t t;
        time(&t);
        char buff[100];
        strcpy(buff, ctime(&t));
        sendto(sockFd, buff, strlen(buff) + 1, 0, (const struct sockaddr *)&clientAddr, sockLen);
        printf("Data sent to client IP:%s port:%d\n", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
    }
    close(sockFd);

    return 0;
}