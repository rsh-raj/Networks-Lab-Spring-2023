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
void sendWrapper(int sockfd, char *buff);
int main(int argc, char **argv)
{
    //normal tcp routine to connect to server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0), serverPort = 20000;
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
    inet_aton("192.168.50.177", &serverAddr.sin_addr);
    socklen_t socklen = sizeof(serverAddr);
    if (connect(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
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
    // send(sockfd, userName, strlen(userName) + 1, 0);
    sendWrapper(sockfd, userName);
    resetBuffer(buff, 50);
    receiveWrapper(sockfd, &buff);
    //if user is found
    if (strcmp(buff, "FOUND") == 0)
    {
        printf("Login Successful!!!\n");
        while (1)
        {
            //get command from user and send it to server
            printf("Enter your command$ ");
            char command[100];
            fgets(command, 1000, stdin);
            command[strlen(command) - 1] = '\0';
            sendWrapper(sockfd, command);
            if (strcmp(command, "exit") == 0)
                break;
            char *result = (char *)malloc(10000 * sizeof(char));
            resetBuffer(result, 10000);
            receiveWrapper(sockfd, &result);
            //print the result
            if (strcmp(result,"$$$$")==0)
            {
                printf("Invalid command\n");
            }
            else if (strcmp("####",result)==0)
            {
                printf("Error in running command\n");
            }
            else
                printf("%s\n", result);
          
        }
    }
    //if user is not found
    else
    {
        printf("Unable to validate user :(, closing the connection");
    }

    close(sockfd);

    return 0;
}
//wrapper function to receive data from server in chunks of 20 bytes
void receiveWrapper(int sockfd, char **buff)
{
    char recvBuffer[50];
    while (1)
    {
        resetBuffer(recvBuffer, 50);
        int x = recv(sockfd, recvBuffer, 20, 0);
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
//wrapper function to send data to server in chunks of max (50) bytes
void sendWrapper(int sockfd, char *buff)
{

    int i = 0;
    while (i <= strlen(buff))
    {
        char sendBuff[50];
        resetBuffer(sendBuff, 50);
        int k = 0;
        int j;
        for (j = i; j <= strlen(buff) && j < i + 49; j++)
        {
            sendBuff[k] = buff[j];
            k++;
        }
        if (sendBuff[k - 1] == '\0')
        {
            send(sockfd, sendBuff, strlen(sendBuff) + 1, 0);
        }
        else
        {
            send(sockfd, sendBuff, strlen(sendBuff), 0);
        }

        i += 49;
    }
}
//function to reset buffer
void resetBuffer(char *buff, int MAX_LEN)
{
    for (int i = 0; i < MAX_LEN; i++)
    {
        buff[i] = '\0';
    }
}
