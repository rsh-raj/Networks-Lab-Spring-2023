#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
int main()
{
    int sockFd, port = 20000;
    if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("unable to create a socket :(");
        exit(1);
    }
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    inet_aton("127.0.0.1", &sockAddr.sin_addr);
    socklen_t sockLen = sizeof(sockAddr);
    if (bind(sockFd, (const struct sockaddr *)&sockAddr, sockLen) < 0)
    {
        perror("Unable to bind the socket to given address :(");
        exit(1);
    }
    printf("Server is running at port:%d\n",port);
    int clientPort=20001;
    struct sockaddr_in clientAddr;
    clientAddr.sin_family=AF_INET;
    clientAddr.sin_port=htons(clientPort);
    inet_aton("127.0.0.1",&clientAddr.sin_addr);

    time_t t;
    time(&t);
    char buff[100];
    strcpy(buff,ctime(&t));
    sendto(sockFd,buff,strlen(buff)+1,0,(const struct sockaddr*)&clientAddr,sockLen);
    printf("Data sent to client");
    close(sockFd);

    return 0;
}