/*
    Name : Aditya Choudhary
    Roll No: 20CS10005
    Course: Networks Lab
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

char **tokenize_command(char *cmd)
{
    int index = 0;
    char temp[500];

    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(100 * sizeof(char));

    int cnt = 0;
    int flag = 0;
    int space = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        // remove the starting spaces
        if (flag == 0 && cmd[i] == ' ') continue;
        flag = 1;

        cnt = 0;
        if(space == 1 && cmd[i] == ' ') continue;
        else if(cmd[i] == ' '){
            temp[cnt++] = cmd[i];
            space = 1;
            continue;
        }

        // index for populating the array
        while (!(cmd[i] == ' ' && cmd[i-1] != '\\'))
        {   
            if(cmd[i] == '\0') break;
            if(cmd[i] == '\\'){
                i++;
                // skipping the back slash
                temp[cnt++] = cmd[i++];
                continue;
            }
            temp[cnt++] = cmd[i++];
            //added random
        }

        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        cmdarr = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        cmdarr[index] = (char *)malloc(100 * sizeof(char));

        if (cmd[i] == '\0')  break;
    }

    cmdarr[index] = NULL;
    return cmdarr;
}

void getIPandPort(char **tokens, char *IP, int *portnum){

    int flag = 0;
    int index = 0;
    int cnt = 0;
    char port[10];
    *portnum = 80;    // default port number

    for (int i = 0; tokens[1][i] != '\0'; i++)
    {
        if(tokens[1][i] == '/' && flag == 1){
            // IP begins
            i++;
            while(tokens[1][i] != '/'){
                IP[index++] = tokens[1][i++];
            }
            IP[index++] = '\0';
        }

        else if(tokens[1][i] == '/') flag = 1;

        else if(tokens[1][i] == ':' && tokens[1][i+1] != '/'){
            flag = 0;
            // port number begins
            i++;
            while(tokens[1][i] != '\0'){
                port[cnt++] = tokens[1][i++];
            }
            port[cnt++] = '\0';
            *portnum = atoi(port);
        }

        else flag = 0;
    }
    
}

int establish_connection(int port, char *IP){

    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
    }

    serv_addr.sin_family	= AF_INET;
    inet_aton(IP, &serv_addr.sin_addr);
    serv_addr.sin_port	= htons(port);

    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
        perror("Unable to connect to server\n");
        exit(0);
    }
}

int main(){
    int sockfd;
    int port;
    struct sockaddr_in serv_addr;
    char cmd[1000];
    char IPaddress[20];

    while(1){
        // initiate the command prompt and take the command
        printf("MyOwnBrowser > ");
        fgets(cmd, 1000, stdin);
        cmd[strlen(cmd)-1] = '\0';  
        printf("Command entered : %s\n", cmd);

        // tokenize the command and form an array of strings
        char **tokens = tokenize_command(cmd);
        int i = 0;
        while(tokens[i]){
            printf("%s\n", tokens[i]);
            i++;
        }

        if(!strcmp(tokens[0], "QUIT")) break;

        // parse the tokenized array to get the server IP and port
        getIPandPort(tokens, IPaddress, &port);
        printf("Port is : %d and IP is %s\n", port, IPaddress);

        // sockfd = establish_connection(port, IPaddress);

        // form the message to be sent to the server


    }
}