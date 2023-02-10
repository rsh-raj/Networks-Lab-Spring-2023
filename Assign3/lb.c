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
#define _GNU_SOURCE
#include <signal.h>
#include <poll.h>
void resetBuffer(char *buff, int buffSize);
char *connectToServer(int serverPort, int *serverSockfd, int serverNumber);
void receiveWrapper(int sockfd, char **buff);

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Please enter the port numbers of load balancer and two servers\n");
        exit(0);
    }
    int lbPort = atoi(argv[1]), server1Port = atoi(argv[2]), server2Port = atoi(argv[3]);
    int waitSockfd, clientSockfd, server1Sockfd, server2Sockfd;
    if ((waitSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create a socket\n");
    }
    struct sockaddr_in lbAddr;
    lbAddr.sin_family = AF_INET;
    lbAddr.sin_port = htons(lbPort);
    inet_aton("127.0.0.1", &lbAddr.sin_addr);
    socklen_t lbAddrLen = sizeof(lbAddr);
    if (bind(waitSockfd, (const struct sockaddr *)&lbAddr, lbAddrLen) < 0)
    {
        perror("Bind failed:(");
        exit(0);
    }
    if ((listen(waitSockfd, 5)) < 0)
    {
        perror("Listen failed:(");
        exit(0);
    }
    struct pollfd pollInfo;
    pollInfo.fd = waitSockfd;
    pollInfo.events = POLLIN;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int loadServer1 = 0, loadServer2 = 0;
    int pollTimeout = 0;
    time_t timeBeforePoll;
    while (1)
    {
        int pollResult = poll(&pollInfo, 1, pollTimeout); // poll for pollTimeout seconds
        if (pollResult == -1)
        {
            perror("Poll failed:(");
            exit(0);
        }
        if (pollResult == 0)
        {
            printf("Polled for %d seconds\n", pollTimeout / 1000);
            time(&timeBeforePoll);
            pollTimeout = 5000;
            if ((server1Sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Unable to create socket :(");
                return 0;
            }
            if ((server2Sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("Unable to create socket :(");
                return 0;
            }

            char *ip1 = connectToServer(server1Port, &server1Sockfd, 1);
            char *ip2 = connectToServer(server2Port, &server2Sockfd, 2);
            char *loadMsg = "Send load";
            send(server1Sockfd, loadMsg, strlen(loadMsg) + 1, 0);
            char *loadBuff = (char *)malloc(100 * sizeof(char));
            resetBuffer(loadBuff, 100);
            // recv(server1Sockfd, loadBuff, 100, 0);
            receiveWrapper(server1Sockfd, &loadBuff);
            loadServer1 = atoi(loadBuff);
            printf("Load received form server1 %s: %d\n", ip1,loadServer1);
            send(server2Sockfd, loadMsg, strlen(loadMsg) + 1, 0);
            // recv(server2Sockfd, loadBuff, 100, 0);
            resetBuffer(loadBuff, 100);
            receiveWrapper(server2Sockfd, &loadBuff);
            loadServer2 = atoi(loadBuff);
            printf("Load received form server2 %s: %d\n", ip2,loadServer2);
            close(server1Sockfd);
            close(server2Sockfd);
            free(loadBuff);
        }
        else if (pollResult > 0)
        {
            time_t timeAfterPoll;
            time(&timeAfterPoll);
            pollTimeout = 5000 - difftime(timeAfterPoll,timeBeforePoll) * 1000;
            if ((clientSockfd = accept(waitSockfd, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0)
            {
                perror("Accept failed:(");
                exit(0);
            }
            if (fork() == 0)
            {
                printf("Client connected\n");
                char *timeMsg = "Send time";
                char *timeBuff = (char *)malloc(100 * sizeof(char));

                if (loadServer1 < loadServer2)
                {
                    if ((server1Sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    {
                        perror("Unable to create socket :(");
                        return 0;
                    }
                    char *ip=connectToServer(server1Port, &server1Sockfd, 1);
                    printf("Sending client request to server1 %s\n",ip);
                    send(server1Sockfd, timeMsg, strlen(timeMsg) + 1, 0);
                    resetBuffer(timeBuff, 100);
                    // recv(server1Sockfd, timeBuff, 100, 0);
                    receiveWrapper(server1Sockfd, &timeBuff);
                    printf("Time received from server1: %s\n", timeBuff);
                    send(clientSockfd, timeBuff, strlen(timeBuff) + 1, 0);
                    close(server1Sockfd);
                }
                else
                {
                    if ((server2Sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    {
                        perror("Unable to create socket :(");
                        return 0;
                    }
                    char *ip=connectToServer(server2Port, &server2Sockfd, 2);
                    printf("Sending client request to server2 %s\n",ip);
                    send(server2Sockfd, timeMsg, strlen(timeMsg) + 1, 0);
                    resetBuffer(timeBuff, 100);
                    // recv(server2Sockfd, timeBuff, 100, 0);
                    receiveWrapper(server2Sockfd, &timeBuff);
                    printf("Time received from server2: %s\n", timeBuff);
                    send(clientSockfd, timeBuff, strlen(timeBuff) + 1, 0);
                    close(server2Sockfd);
                }
                close(clientSockfd);
                free(timeBuff);
                exit(0);
            }
        }
    }

    return 0;
}
char *connectToServer(int serverPort, int *serverSockfd, int serverNumber)
{
    struct sockaddr_in serveAddr;
    serveAddr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serveAddr.sin_addr);
    serveAddr.sin_port = htons(serverPort);
    if (connect(*serverSockfd, (struct sockaddr *)&serveAddr, sizeof(serveAddr)) < 0)
    {
        printf("Unable to connect to the server%d:(\n", serverNumber);
        perror("");
        exit(0);
    }
    return inet_ntoa(serveAddr.sin_addr);
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