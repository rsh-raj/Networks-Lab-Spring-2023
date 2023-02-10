#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    int port = 20000;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket :(");
        return 0;
    }
    struct sockaddr_in serveAddr;
    serveAddr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serveAddr.sin_addr);
    serveAddr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *)&serveAddr, sizeof(serveAddr)) < 0)
    {
        perror("Unable to connect to the server :(\n");
        exit(0);
    }
    char buff[100];
    for (int i = 0; i < 100; i++)
        buff[i] = '\0';
    char x = recv(sockfd, buff, 100, 0);
    printf("Current date and time: %s", buff);

    return 0;
}