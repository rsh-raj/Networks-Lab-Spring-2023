#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
void receiveWrapper(int sockfd, char **buff);
void resetBuffer(char *buff, int size);
int main(int argc, char **argv)
{
    int port = 2000;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    else
    {
        printf("Usage client<server port>");
        exit(0);
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
    char *buff = malloc(100 * sizeof(char));
    for (int i = 0; i < 100; i++)
        buff[i] = '\0';
    receiveWrapper(sockfd, &buff);
    printf("Current date and time: %s", buff);
    free(buff);

    return 0;
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
void resetBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = '\0';
    }
}