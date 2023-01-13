#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
void resetBuffer(char buff[])
{
    for (int i = 0; i < 1000000; i++)
        buff[i] = '\0';
}
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
    if ((connect(sockfd, (struct sockaddr *)&serveAddr, sizeof(serveAddr))) < 0)
    {
        perror("Unable to connect to the server :(\n");
        exit(0);
    }
    char buff[1000000];
    while (1)
    {
        char expression[100000], result[100];
        resetBuffer(buff);
        // printf("Enter the expression you want to evaluate and press the enter key when you are done\n");
        char ch = '1';
        int i = 0;
        while (ch != '\n')
        {

            scanf("%c", &ch);
            buff[i++] = ch;
            if (i > 1000000)
            {
                send(sockfd, buff, strlen(buff) + 1, 0);
                i = 0;
                resetBuffer(buff);
            }
        }
        if (i != 0)
            send(sockfd, buff, strlen(buff) + 1, 0);
        if (buff[0] == '-' && buff[1] == '1' && buff[2] == '\n')
            break;
        if ((recv(sockfd, result, 20, 0)) < 0)
        {
            perror("Unable to receive the result");
        }
        printf("Result: %s\n", result);
    }

    close(sockfd);
    return 0;
}