#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <sys/mman.h>
#include <signal.h>
#include <termios.h>
#include <readline/history.h>
#include <readline/readline.h>

struct Job
{
    int id;
    int pid;
    char *cmd;
};

static int numJobs = 0;
static char *env;
static char *dir;
static char *curAction;
static struct Job jobs[100];
char *clearWhitespace(char *str)
{
    char *end;

    while (isspace(*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;
    *(end + 1) = 0;

    return str;
}

void createPipe(){
    int pipefd[2];
    pid_t pid, pid2;
    pipe(pipefd);
    pid = fork();
    if (pid == 0)
    {   
        dup2(pipefd[1], STDOUT_FILENO);
        clearWhitespace(command);
        close(pipefd[0]);
        close(pipefd[1]);
        exit(NULL);
    }
    pid2 = fork();
    if(pid2 == 0){
        dup2(pipefd[0], STDOUT_FILENO);
        clearWhitespace(command);
        close(pipefd[0]);
        close(pipefd[1]);
        exit(NULL);
    }
    
}


void performAction()
{
    // char *command;
    // char *args[20];
    // for (int i = 0; i < 20; i++)
    // {
    //     args[i] = NULL;
    // }
    // int numArgs = 0;
    // char *input = strdup(curAction);
    // command = strtok(curAction, " ");
    // while (command != NULL)
    // {
    //     args[numArgs] = command;
    //     command = strtok(NULL, " ");
    //     numArgs++;
    // }
    // char *Args[19];
    // for (int i = 0; i < 19; i++)
    // {
    //     Args[i] = NULL;
    // }
    // for (int i = 1; i < 20; i++)
    // {
    //     if (args[i] != NULL)
    //     {
    //         Args[i - 1] = args[i];
    //     }
    // }

    // if (strcmp("cd", args[0]) == 0)//CD
    // {
    //     cd(args[1]);
    // }
    // else if (strcmp(args[0], "set") == 0)// Set Path
    // {
    //     setPath(args[1]);
    // }
    // else if (strcmp(args[0], "jobs") == 0)//Show all jobs
    // {
    //     showJobs();
    // }
    // else if (strchr(input, '|') != NULL)//Pipe creation
    // {
    //     makePipe();
    // }
    // else if (strchr(input, '&') != NULL)//Background Process
    // {
    //     process();
    // }
    // else if (strstr(input, "kill") != NULL)//Kill Job
    // {
    //     int pid = (int) strtol(args[2], NULL, 0);
    //     int signal = (int) strtol(args[1], NULL, 0);
    //     printf("Killing pid %d with signal %d\n", pid, signal);
    //     kill(pid, -9);
    // }
    // else if (strchr(input, '<') != NULL)//file in
    // {
    //     fileIN(args);
    // }
    // else if (strchr(input, '>') != NULL)//file out
    // {
    //     fileOUT(args);
    // }
    // else//Run executables
    // {
    //     execute(args);
    // }
}


int main(int argc, char *argv[]){
    printf("EECS678-QUASH\n\n");
    char *action, prompt[128];
    env = getenv("USER");
    dir = getcwd(NULL, 1024);
    numJobs = 0;

    while (1)
    {
        snprintf(prompt, sizeof(prompt), "%s:%s> ", env, dir);
        action = readline(prompt);
        
        action = clearWhitespace(action);
        if (strcmp("exit", action) != 0 && strcmp("quit", action) != 0)
        {
            if (strlen(action) > 1)
            {
                action = clearWhitespace(action);
                curAction = action;
                performAction();
            }
        }
        else
        {
            break;
        }
        free(action);
    }

    return 0;
    
    
}