/*The tcp concurrent server*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
//function headers
int validateUser(char **users, int totalUsers, char userName[]);
void sendWrapper(int sockfd, const char *buff);
void executeCommand(char command[], char directory[], int);
void resetBuffer(char *buff, int MAX_LEN);
void receiveWrapper(int sockfd, char **buff);
int main(int argc, char **argv)
{
    //read users from file
    FILE *filePointer;
    filePointer = fopen("users.txt", "r");
    if (!filePointer)
    {
        perror("Unable to open the file :(");
        exit(0);
    }
    char **users = (char **)malloc(sizeof(char *));
    users[0] = (char *)malloc(26 * sizeof(char));

    char ch;
    int noOfUsers = 0;
    int idx = 0;

    while (1)
    {
        ch = fgetc(filePointer);
        if (ch == '\n')
        {
            noOfUsers++;
            users = (char **)realloc(users, (noOfUsers + 1) * sizeof(char *));
            users[noOfUsers] = (char *)malloc(26 * sizeof(char));
            idx = 0;
            continue;
        }
        else if (ch == EOF)
            break;
        users[noOfUsers][idx++] = ch;
    }

    //normal tcp routine
    int sockfd, serverPort = 20000, newSockfd;
    if (argc > 1)
        serverPort = atoi(argv[1]);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create a socket :(");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serverAddr, clientAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = serverPort;
    inet_aton("192.168.50.177", &serverAddr.sin_addr);
    // serverAddr.sin_addr.s_addr = INADDR_ANY;
    socklen_t socklen = sizeof(serverAddr);
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, socklen) < 0)
    {
        perror("Bind failed :(");
        exit(EXIT_FAILURE);
    }
    if (listen(sockfd, 5) < 0)
    {
        perror("Failed to listen :(");
        exit(EXIT_FAILURE);
    }
    socklen = sizeof(clientAddr);
    while (1)
    {
        printf("Waiting for client to connect \n");
        if ((newSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &socklen)) < 0)
        {
            perror("unable to accept new connection from client :(");
            exit(0);
        }
        //fork a child process to handle the client
        // if (fork() == 0)
        // {
        //     close(sockfd);
        //     printf("A client with following address have connected: IP:%s Port:%d\n", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
        //     char *loginMessage = "LOGIN:";
        //     send(newSockfd, loginMessage, strlen(loginMessage) + 1, 0);
        //     char *buff = (char *)malloc(26 * sizeof(char));
        //     resetBuffer(buff, 26);
        //     receiveWrapper(newSockfd, &buff);
        //     //validate the user
        //     if (validateUser(users, noOfUsers, buff))
        //     {
        //         char *userMessage = userMessage = "FOUND";
        //         send(newSockfd, userMessage, strlen(userMessage) + 1, 0);
        //         while (1)
        //         {
        //          //receive the command from the client
        //             char *command = (char *)malloc(205 * sizeof(char));
        //             resetBuffer(command, 205);
        //             receiveWrapper(newSockfd, &command);
        //             char commandName[5], directory[1000];
        //             resetBuffer(commandName, 5);
        //             resetBuffer(directory, 1000);
        //             int j = 0;
        //             while (j < strlen(command) && command[j] != ' ')
        //             {
        //                 commandName[j] = command[j];
        //                 j++;
        //             }
        //             while (command[j++] == ' ' && j < strlen(command))
        //                 ;

        //             for (int i = j - 1, k = 0; i < strlen(command); i++)
        //                 directory[k++] = command[i];
        //             if (strcmp(command, "exit") == 0)
        //             {
        //                 close(newSockfd);
        //                 exit(0);
        //             }
        //             printf("command: %s\n", commandName);
        //             //execute the command
        //             executeCommand(commandName, directory, newSockfd);
        //         }
        //     }
        //     else //user not found
        //     {
        //         char *userMessage = userMessage = "NOT FOUND";
        //         send(newSockfd, userMessage, strlen(userMessage) + 1, 0);
        //     }
        // }
    }
    // free the users array
    for (int i = 0; i < noOfUsers; i++)
        free(users[i]);
    free(users);
    return 0;
}
//function to validate the user
int validateUser(char **users, int totalUsers, char userName[])
{
    for (int i = 0; i < totalUsers; i++)
    {
        if (strcmp(users[i], userName) == 0)
        {
            return 1;
        }
    }
    return 0;
}
//function to execute the command
void executeCommand(char command[], char directory[], int newSockfd)
{
    const char *errorInRunning = "####";
    //if the command is pwd then send the current working directory
    if (strcmp(command, "pwd") == 0)
    {

        char buff[PATH_MAX];
        if (getcwd(buff, sizeof(buff)))
        {
            sendWrapper(newSockfd, buff);
        }
        else
        {
            sendWrapper(newSockfd, errorInRunning);
        }
        printf("%s\n", buff);
        return;
    }
    //if the command is dir then send the list of files in the directory
    else if (strcmp(command, "dir") == 0)
    {
        struct dirent *fileStruct;
        if (directory[0] == '\0')
            directory[0] = '.';
        DIR *drctory = opendir(directory);
        //buffer to store the result of dir command
        char result[10000];
        resetBuffer(result, 10000);
        if (drctory != NULL)
        {
            while ((fileStruct = readdir(drctory)) != NULL)
            {
                strcat(result, fileStruct->d_name);
                strcat(result, "\n");
            }
            printf("Result: %s", result);
            sendWrapper(newSockfd, result);
        }
        else
        {
            sendWrapper(newSockfd, errorInRunning);
            printf("%s\n", errorInRunning);
        }
        closedir(drctory);
        return;
    }
    //if the command is cd then change the directory
    else if (strcmp(command, "cd") == 0)
    {
        char *success = "Directory changed successfully!";
        if (chdir(directory) < 0)
        {
            sendWrapper(newSockfd, errorInRunning);
            printf("%s\n", errorInRunning);
        }
        else
        {
            sendWrapper(newSockfd, success);
            printf("%s\n", success);
        }
        return;
    }
    char *unrecognisedCommand = "$$$$";
    sendWrapper(newSockfd, unrecognisedCommand);
    return;
}
//helper function to reset the buffer
void resetBuffer(char *buff, int MAX_LEN)
{
    for (int i = 0; i < MAX_LEN; i++)
    {
        buff[i] = '\0';
    }
}
//helper function to send the data in chunks(50 bytes)
void sendWrapper(int sockfd, const char *buff)
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
//helper function to receive the data in chunks(50 bytes)
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