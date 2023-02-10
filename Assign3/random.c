if ((waitSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
{
    perror("Unable to create a socket :(");
    exit(0);
}
struct sockaddr_in lbAddr, clientAddr;
lbAddr.sin_addr.s_addr = INADDR_ANY;
lbAddr.sin_family = AF_INET;
lbAddr.sin_port = htons(lbPort);
if (bind(waitSockfd, (struct sockaddr *)&lbAddr, sizeof(lbAddr)) < 0)

{
    perror("Unable to bind the socket to local address :(");
    exit(0);
}
printf("Load balancer is listening at %d\n", lbPort);
if (listen(waitSockfd, 5) < 0)
{
    perror("Unable to listen");
    exit(0);
}
int cliLen = sizeof(clientAddr);
char buff[100];
time_t t;
struct pollfd pollInfo;
pollInfo.fd = waitSockfd;
pollInfo.events = POLLIN;

// Create a socket for server1
if ((server1Sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
{
    perror("Unable to create a socket :(");
    exit(0);
}
struct sockaddr_in server1Addr;
server1Addr.sin_addr.s_addr = inet_aton("127.0.0.1", &server1Addr.sin_addr);
server1Addr.sin_family = AF_INET;
server1Addr.sin_port = htons(server1Port);
// Create a socket for server2
if ((server2Sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
{
    perror("Unable to create a socket :(");
    exit(0);
}
struct sockaddr_in server2Addr;
server2Addr.sin_addr.s_addr = inet_aton("127.0.0.1", &server2Addr.sin_addr);
server2Addr.sin_family = AF_INET;
server2Addr.sin_port = htons(server2Port);
int lastKnownLoadServer1 = 0, lastKnownLoadServer2 = 0;
if (connect(server1Sockfd, (struct sockaddr *)&server1Addr, sizeof(server1Addr)) < 0)
{
    perror("Unable to connect to server1 :(");
    exit(0);
}
else
{
    printf("Connected to server1 %s %d  ", inet_ntoa(server1Addr.sin_addr), server1Port);
}
char *loadMsg = "Send load";
if (send(server1Sockfd, loadMsg, strlen(loadMsg) + 1, 0) < 0)
{
    perror("Unable to send load request to server1 :(");
    exit(0);
}
// while (1)
// {
//     int pollResult = poll(&pollInfo, 1, 5000); // poll for 5 seconds
//     if (pollResult == -1)
//     {
//         perror("Poll error");
//         exit(0);
//     }
//     char *loadMsg="Send load",*timeMsg="Send time";
//     if (pollResult == 0)
//     {
//         printf("No new client in 5 seconds, checking load of servers\n");
//         // Check load of servers
//         resetBuffer(buff);
//         if (connect(server1Sockfd, (struct sockaddr *)&server1Addr, sizeof(server1Addr)) < 0)
//         {
//             perror("Unable to connect to server1 :(");
//             exit(0);
//         }
//         printf("Connected to server1 %s %d  ", inet_ntoa(server1Addr.sin_addr), server1Port);
//         if (send(server1Sockfd, loadMsg, strlen(loadMsg)+1, 0) < 0)
//         {
//             perror("Unable to send load request to server1 :(");
//             exit(0);
//         }
//         if (recv(server1Sockfd, buff, 100, 0) < 0)
//         {
//             perror("Unable to receive load from server1 :(");
//             exit(0);
//         }
//         int loadServer1 = atoi(buff);
//         printf("Load received from server1 %s %d", inet_ntoa(server1Addr.sin_addr), loadServer1);
//         resetBuffer(buff);
//         if (connect(server2Sockfd, (struct sockaddr *)&server2Addr, sizeof(server2Addr)) < 0)
//         {
//             perror("Unable to connect to server2 :(");
//             exit(0);
//         }
//         if (send(server2Sockfd, loadMsg,strlen(loadMsg)+1, 0) < 0)
//         {
//             perror("Unable to send load request to server2 :(");
//             exit(0);
//         }
//         if (recv(server2Sockfd, buff, 100, 0) < 0)
//         {
//             perror("Unable to receive load from server2 :(");
//             exit(0);
//         }
//         int loadServer2 = atoi(buff);
//         printf("Load received from server2 %s %d", inet_ntoa(server2Addr.sin_addr), loadServer2);
//     }

//     if (pollResult > 0)
//     {
//         resetBuffer(buff);

//         if ((clientSockfd = accept(waitSockfd, (struct sockaddr *)&clientAddr, (socklen_t *)&cliLen)) < 0)
//         {
//             perror("Unable to accept the connection :(");
//             exit(0);
//         }
//         printf("A new client connected\n");
//         if (lastKnownLoadServer1 < lastKnownLoadServer2)
//         {
//             if (connect(server1Sockfd, (struct sockaddr *)&server1Addr, sizeof(server1Addr)) < 0)
//             {
//                 perror("Unable to connect to server1 :(");
//                 exit(0);
//             }
//             if (send(server1Sockfd, timeMsg, strlen(timeMsg)+1, 0) < 0)
//             {
//                 perror("Unable to send date request to server1 :(");
//                 exit(0);
//             }
//             if (recv(server1Sockfd, buff, 100, 0) < 0)
//             {
//                 perror("Unable to receive date from server1 :(");
//                 exit(0);
//             }
//         }
//         else
//         {
//             if (connect(server2Sockfd, (struct sockaddr *)&server2Addr, sizeof(server2Addr)) < 0)
//             {
//                 perror("Unable to connect to server2 :(");
//                 exit(0);
//             }
//             if (send(server2Sockfd, timeMsg, strlen(timeMsg), 0) < 0)
//             {
//                 perror("Unable to send date request to server2 :(");
//                 exit(0);
//             }
//             if (recv(server2Sockfd, buff, 100, 0) < 0)
//             {
//                 perror("Unable to receive date from server2 :(");
//                 exit(0);
//             }
//         }
//         if (send(clientSockfd, buff, strlen(buff) + 1, 0) < 0)
//         {
//             perror("Unable to send the message to client :( error");
//         }
//         printf("Date and time sent to client and client disconnected\n");
//         close(clientSockfd);
//     }
// }
