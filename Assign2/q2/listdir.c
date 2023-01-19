#include <stdio.h>
#include <dirent.h>
int main(void)
{
    struct dirent *de; // Pointer for directory entry
    DIR *dr = opendir("/home/rsh-raj");
    if (dr == NULL) // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory");
        return 0;
    }
    while ((de = readdir(dr)) != NULL)
        printf("%s\n", de->d_name);
    closedir(dr);
    return 0;
}




