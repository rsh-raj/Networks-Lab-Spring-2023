// #include <stdio.h>
// #include <unistd.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <stdlib.h>
// #include <sys/stat.h>
// #include <time.h>
// #include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



char **tokenize_command(char *cmd)
{
    // what the fuck is this malloc errror on:
    // GET http://127.0.0.1/home/rsh-raj/Documents/KGP/sem6/networks/Networks-Lab-Spring-2023/Assign1/Assgn-1.pdf:3000
    int index = 0;
    char temp[5000];

    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(1000 * sizeof(char));

    int cnt = 0;
    int flag = 0;
    int space = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        // remove the starting spaces
        if (flag == 0 && cmd[i] == ' ')
            continue;
        flag = 1;

        cnt = 0;
        if (space == 1 && cmd[i] == ' ')
            continue;
        else if (cmd[i] == ' ')
        {
            temp[cnt++] = cmd[i];
            space = 1;
            continue;
        }

        // index for populating the array
        while (!(cmd[i] == ' ' && cmd[i - 1] != '\\'))
        {
            if (cmd[i] == '\0')
                break;
            if (cmd[i] == '\\')
            {
                i++;
                // skipping the back slash
                temp[cnt++] = cmd[i++];
                continue;
            }
            temp[cnt++] = cmd[i++];
            // added random
        }

        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        char **tmp = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        if (tmp == NULL)
        {
            printf("realloc failed:367\n");
            exit(1);
        }
        cmdarr = tmp;
        cmdarr[index] = (char *)malloc(1000 * sizeof(char));

        if (cmd[i] == '\0')
            break;
    }

    cmdarr[index] = NULL;
    return cmdarr;
}

char * modifydate(int changeday){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    tm.tm_mday += changeday;
    mktime(&tm);
    char buf[50];
    strcpy(buf,asctime(&tm));

    buf[strlen(buf)-1] = '\0';
    // printf("%s\n", buf);

    char **temp = tokenize_command(buf);

    // Now HTTP formatting
    char *final = (char *)malloc(100*sizeof(char));
    strcpy(final, temp[0]);
    strcat(final,", ");
    strcat(final,temp[2]);
    strcat(final," ");
    strcat(final,temp[1]);
    strcat(final," ");
    strcat(final,temp[4]);
    strcat(final," ");
    strcat(final,temp[3]);
    strcat(final," ");
    strcat(final,"IST");

    // printf("Final date : %s\n", final);
    return final;
}


int main(){
    char buf[200];
    time_t tx;   // not a primitive datatype
    time(&tx);
    strcpy(buf,ctime(&tx));
    printf("This program has been writeen at (date and time): %s", buf);
    strcpy(buf, "If-Modified-Since: ");
    strcat(buf, modifydate(0));
    strcat(buf, "\r\n");
    printf("%s", buf);
    // printf("%d\n", tm.tm_year + 1900);
    // printf("%d\n", tm.tm_mon + 1);
    // printf("%d\n", tm.tm_mday);
    // printf("%d\n", tm.tm_hour);
    // printf("%d\n", tm.tm_min);
    // printf("%d\n", tm.tm_sec);
    // struct stat m;
    // struct tm dt = *(gmtime(&m.st_mtime));
    // stat("MyBrowser.c", &m);
}
