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
#define CLOSE 0
#define OPEN 1
int receiveAndEvaluate(int *);
int isaOperator(char ch);
void resetBuffer(char buff[]);
int query=0;
int main(int argc, char **argv)
{
    // setting the default port to 2000 if no port is provided
    int port = 20000;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    int sockfd, newSockfd;
    // creating a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create a socket :(");
        exit(0);
    }
    // binding the socket to the local address
    struct sockaddr_in serveAddr, clientAddr;
    serveAddr.sin_addr.s_addr = INADDR_ANY;
    serveAddr.sin_family = AF_INET;
    serveAddr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&serveAddr, sizeof(serveAddr)) < 0)
    {
        perror("Unable to bind the socket to local address :(");
        exit(0);
    }
    // listening for connections
    printf("Server is listening at %d\n", port);
    if (listen(sockfd, 2) < 0)
    {
        perror("Unable to listen");
        exit(0);
    }
    int cliLen = sizeof(clientAddr);
    // accepting the connection
    while (1)
    {
        if ((newSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &cliLen)) < 0)
        {
            perror("Unable to accept the connection :(");
            exit(0);
        }
        printf("Client connected\n");
        // while the client is connected to the server receive the expression and evaluate it
        int clientConnection = OPEN;
        query=0;
        while (clientConnection)
        {
            clientConnection = receiveAndEvaluate(&newSockfd);
        }
        // close the connection
        close(newSockfd);
        printf("Client disconnected\n");
    }

    return 0;
}
int receiveAndEvaluate(int *newSockfd)
{
    double ans = 0, temp_ans = 0;
    char operator= '+', temp_operator = '+';
    char operand[1000], temp_operand[1000];
    int open = 0;
    // reset the buffers
    resetBuffer(operand);
    resetBuffer(temp_operand);
    while (1)
    {
        // printf("calculating the result for: ");
        char expression[100];
        for (int i = 0; i < 100; i++)
            expression[i] = '\0';
        // receive the expression from the client
        if ((recv(*newSockfd, expression, 100, 0)) < 0)
        {
            perror("Some error occurred while receiving the expression from client");
        }
        // printf("%s\n", expression);
        int n = strlen(expression);
        // if the client sends -1\n then close the connection
        if (n >= 3 && expression[0] == '-' && expression[1] == '1' && expression[2] == '\n')
        {
            return CLOSE;
        }
        for (int i = 0; i < n;)
        {
            // if the expression has a bracket then calculate the result of the bracket and store it in temp_ans
            if (expression[i] == '(')
            {
                i++, open = 1;
            }

            if (expression[i] == ' ')
            {
                i++;
                continue;
            }
            while (open == 1 && i < n)
            {
                // printf("Calculating the result of a bracket(temp_ans)\n");
                if (expression[i] == ' ')
                {
                    i++;
                    continue;
                }
                int j = strlen(temp_operand);
                while (!isaOperator(expression[i]) && i < n && expression[i] != '\n' && expression[i] != ')')
                {
                    temp_operand[j++] = expression[i++];
                }
                if (isaOperator(expression[i]) || expression[i] == ')')
                {
                    char *end;
                    // printf("Performing operation on temp_operator:%c temp_operand:%s\n", temp_operator, temp_operand);
                    if (temp_operator == '+')
                        temp_ans += strtod(temp_operand, &end);
                    else if (temp_operator == '-')
                        temp_ans -= strtod(temp_operand, &end);
                    else if (temp_operator == '*')
                        temp_ans *= strtod(temp_operand, &end);
                    else if (temp_operator == '/')
                        temp_ans /= strtod(temp_operand, &end);
                    if (expression[i] == ')')
                    {
                        open = -1;
                    }
                    temp_operator = expression[i++];
                    resetBuffer(temp_operand);
                }

                if (open == -1)
                {
                    break;
                }
                // printf("temp_ans after this step: %f\n", temp_ans);
            }
            // if closing bracket haven't been encountered then continue
            if (open == 1)
                continue;
            // if closing bracket have been encountered then store the result of the bracket in operand
            else if (open == -1)
            {
                // printf("converting temp_ans(bracket's result) into second operand :%f\n", temp_ans);
                sprintf(operand, "%f", temp_ans);
                temp_ans = 0;
                for (int i = 0; i < 1000; i++)
                    temp_operand[i] = '\0';
                temp_operator = '+';
            }
            // if operand doesn't contains the value of the bracket then fill it with the value of next operand
            if (open != -1)
            {
                int j = strlen(operand);
                while (!isaOperator(expression[i]) && i < n && expression[i] != '\n')
                {
                    // printf("j: %d\n", j);
                    operand[j++] = expression[i++];
                }
            }
            // if the expression has a operator then perform the operation on the operand and store the result in ans
            if (isaOperator(expression[i]) || expression[i] == '\n' || open == -1)
            {
                char *end;
                // printf("Performing operation on operator:%c operand:%s\n", operator, operand);
                if (open == -1)
                    open = 0;
                if (operator== '+')
                    ans += strtod(operand, &end);
                else if (operator== '-')
                    ans -= strtod(operand, &end);
                else if (operator== '*')
                    ans *= strtod(operand, &end);
                else if (operator== '/')
                    ans /= strtod(operand, &end);
                operator= expression[i++];
                resetBuffer(operand);
            }
            // printf("answer after this step: %f\n", ans);
        }
        // if the expression has a newline character then send the result to the client
        if (expression[n - 1] == '\n')
        {
            char res[20];
            //handling the negative zero case
            if(ans<0.00000001 && ans>-0.00000001){
                ans = 0;
            }
            sprintf(res, "%f", ans);
            if ((send(*newSockfd, res, strlen(res) + 1, 0)) < 0)
            {
                perror("Some error occurred while sending the result to client");
            }
            printf("Result for query no %d ans: %f sent to client\n\n\n\nWaiting for new expression\n",query++,ans);

            return OPEN;
        }
    }
}
// helper function to reset the buffer
void resetBuffer(char buff[])
{
    for (int i = 0; i < 1000; i++)
        buff[i] = '\0';
}
// helper function to check if the character is an operator
int isaOperator(char ch)
{
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}