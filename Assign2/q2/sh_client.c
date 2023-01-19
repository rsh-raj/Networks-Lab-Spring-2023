#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
void resetBuffer(char *buff, int MAX_LEN);
void receiveWrapper(int sockfd, char **buff);
int main(int argc, char **argv)
{

    int sockfd = socket(AF_INET, SOCK_STREAM, 0), serverPort = 2000;
    if (argc > 1)
        serverPort = atoi(argv[1]);
    if (sockfd < 0)
    {
        perror("Unable to create a socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = serverPort;
    inet_aton("127.0.0.1", &serverAddr.sin_addr);
    socklen_t socklen = sizeof(serverAddr);
    if (connect(sockfd, (const struct sockaddr *)&serverAddr, socklen) < 0)
    {
        perror("Unable to connect to server :(");
        exit(EXIT_FAILURE);
    }
    char *buff = (char *)malloc(50 * sizeof(char));
    resetBuffer(buff, 50);
    receiveWrapper(sockfd, &buff);
    printf("%s", buff);
    char userName[26];
    scanf("%s", userName);
    getchar(); // delete \n from buffer
    send(sockfd, userName, strlen(userName) + 1, 0);
    resetBuffer(buff, 50);
    receiveWrapper(sockfd, &buff);
    if (strcmp(buff, "FOUND") == 0)
    {
        printf("Login Successful!!!\n");
        while (1)
        {
            printf("Enter your command$ ");
            char command[100];
            fgets(command, 1000, stdin);
            command[strlen(command) - 1] = '\0';
            send(sockfd, command, strlen(command) + 1, 0);
            char *result=(char *)malloc(10000*sizeof(char));
            resetBuffer(result, 10000);
            receiveWrapper(sockfd, &result);
            printf("%s\n", result);
            if (strcmp(command, "exit") == 0)
                break;
        }
    }
    else
    {
        printf("Unable to validate user :(, closing the connection");
    }

    close(sockfd);

    return 0;
}
void receiveWrapper(int sockfd, char **buff)
{
    char recvBuffer[50];
    resetBuffer(recvBuffer, 50);
    recv(sockfd, recvBuffer, 50, 0);
    strcat(*buff, recvBuffer);
    printf("inside wrapper: %s %d\n", recvBuffer, strlen(recvBuffer));
    while (recvBuffer[strlen(recvBuffer)] != '\0')
    {
        resetBuffer(recvBuffer, 50);
        recv(sockfd, recvBuffer, 50, 0);
        strcat(*buff, recvBuffer);
    }
}
void resetBuffer(char *buff, int MAX_LEN)
{
    for (int i = 0; i < MAX_LEN; i++)
    {
        buff[i] = '\0';
    }
}
