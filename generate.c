#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#define INSTALL_DIR "/var/lib/sch/sch-3.6.0"
void ExecuteCommand(char **args,int output)
{
     output=1;
     pid_t pid = fork();
     if( pid < 0)
     {
         printf("Fork failed\n");
         exit(-1);
     }
     else if(pid > 0) // parent
     {
         return;
     }
     else
     {
          if(output==0)
          {
              int fd = open("/dev/null", O_WRONLY);
              dup2(fd,1);
              dup2(fd,2);
          }
          if (execvp(args[0], args) == -1)
              printf("Child exits\n");
          exit(-1);
     }
}

int IsFileExist(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}
int IsDirectoryExist(char *dirpath)
{
    DIR* dir = opendir(dirpath);
    if (dir)
    {
        /* Directory exists. */
        closedir(dir);
        return 1;
    }
    else if (ENOENT == errno)
    {
        /* Directory does not exist. */
        return 0;
    }
    else
    {
         /* opendir() failed for some other reason. */
         return 0;
    }
}
void main(int argc, char *argv)
{
    int status = 0;
    pid_t pid;
    char* args[10];
    char *line;
    char *indata[3];
    size_t len=0;
    int read;
    FILE  *fd = fopen("manifest","rb");
    if(fd == NULL)
    {
        printf("Manifest info file not found\n");
        exit(-1);
    }
    int i=0;
    int j=0;
    while ((read = getline(&line, &len, fd)) != -1) {
        indata[i++] =line; 
        line[strcspn(line, "\n")] = 0;
        line=NULL;
    }

    /*for(j=0;j<i;j++)
    {
        printf("%s",indata[j]);
    }*/   
    args[0] = "mkdir";
    args[1] = indata[1];
    args[2] = NULL;
    ExecuteCommand(args,0);
    pid = wait(&status);

    args[0] = "zip";
    args[1] = "--password";
    args[2] = indata[2];
    args[3] = indata[1];
    args[4] = "sch.tar.gz";
    args[5] = NULL;
    ExecuteCommand(args,0);
    pid = wait(&status);

    char filename[200];
    char filename2[200];
    sprintf(filename,"%s.zip",indata[1]);
    sprintf(filename2,"%s/%s",indata[1],indata[1]);
    args[0] = "mv";
    args[1] = filename;
    args[2] = filename2;
    args[3] = NULL;
    ExecuteCommand(args,0);
    pid = wait(&status);

    fd = fopen("license","wb");
    if(fd == NULL)
    {
         printf("Failed to create license file\n");
         exit(-1);
    }
    char url[400];
    sprintf(url,"%s/blob/master/license-ncu?raw=tru",indata[0]);
    fprintf(fd,"%s\n",url);
    for(j=1;j<i;j++)
       fprintf(fd,"%s\n",indata[j]);
    fclose(fd);

    args[0] = "zip";
    args[1] = "--password";
    args[2] = "gsmp1000";
    args[3] = "license";
    args[4] = "license";
    args[5] = NULL;
    ExecuteCommand(args,0);
    pid = wait(&status);

    sprintf(filename,"%s/license",indata[1]);
    args[0] = "mv";
    args[1] = "license.zip";
    args[2] = filename;
    args[3] = NULL;
    ExecuteCommand(args,0);
    pid = wait(&status);

}
