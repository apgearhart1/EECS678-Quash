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
static char *current;
static struct Job jobs[100];

char *clearWhitespace(char *str){
    char *end;

    while (isspace(*str)){
        str++;
    }
    if (*str == 0){
        return str;
    }
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)){
        end--;
    }
    *(end + 1) = 0;
    return str;
}
void performAction();
void out(char *args){
    int index;
    int out;
    int pos = 0;
    while(pos == 0){
        if(args[index] == ">"){
            pos = index;
        }
        index++;
    }
    int s = 0;
    pid_t pid;
    pid = fork();
    if(pid == 0){
        char* name = args[pos + 1];
        out = open(name, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        dup2(out, STDOUT_FILENO);
        char* in = strtok(current, ">");
        performAction();
        close(out);
        exit(0);
    }else{
        wait(NULL);

    }
}
void ex(char **args){
    pid_t p = fork();
    int stat;
    if (p==0){
        if(strlen(args[0]) > 0){
            if (execvp(args[0], args) < 0){
                fprintf(stderr, "Invalid action\n");
                exit(0);
            }
        }
        else{
            if (execvp(args[0], args) < 0){
                fprintf(stderr, "Invalid action\n");
                exit(0);
            }
        }
    }
    else{
        waitpid(p, &stat, 0);
        if (stat == 1){
            fprintf(stderr, "%s\n", "Status == 1\n");
        }
    }
}
void in(char* args){
    char* cmd = NULL;
    int length = 0;
    ssize_t read = 0;
    FILE* fd;
    pid_t pid = fork();
    if (args[0] == "quash"){
        fd = fopen(args[2], "r");
        do{
            read = getline(&cmd, &length, fd);
            if (pid == 0){
                performAction();
            }else{
                wait(NULL);
            }
            
        } while (read!=-1);
        fclose(fd);

    }else{
        int counter = 0;
        fd = fopen(args[2], "r");
        int index = 0;
        while (index == 0){
            if(args[index] == "<"){
                index = counter;
            }
            counter++;
        }
        while((getline(&cmd, &length, fd) < -1)){
            char* t = strtok(cmd, "\0");
            args[index] = t;
            counter++;
            printf("%s\n", args[index]);
        }
        if (pid == 0)
        {
            ex(args);
        }else{
            wait(NULL);
        }
    }
    
    
}


void killProcess(char* args){
    int pid = (int) strtol(args[2], NULL, 0);
    int signal = (int) strtol(args[1], NULL, 0);
    printf("Killing pid %d with signal %d\n", pid, signal);
    kill(pid, -9);
}
void changeDirectory(char *p){
    if (p == NULL){
        chdir(getenv("HOME"));
        dir = getcwd(NULL, 1024);
    }
    else{
        if (chdir(p) == -1){
            printf(" %s: No such file or directory\n", strerror(errno));
        }
        dir = getcwd(NULL, 1024);
    }
}

void viewProcess(){
    int status = 0;
    pid_t pid;
    pid_t sessionid;
    pid = fork();
    if(pid == 0){
        sessionid = setsid();
        if(sessionid == -1){
            fprintf(stderr, "FAILED");
        }
        printf("Process %d running out of %d processes\n", getpid(), numJobs + 1);


        char* background = strdup(current);
        background[strlen(current) - 1] = 0;
        current = background;

        performAction();
        printf("Process %d is done\n", getpid());
        kill(getpid(), -9);
        exit(0);
    }else{
        struct Job new_job;
        new_job.id = numJobs;
        new_job.pid = pid;
        new_job.cmd = current;
        
        jobs[numJobs] = new_job;
        numJobs++;
        while (waitpid(pid, NULL, WNOHANG | WEXITED) > 0)
        {
            printf("%d", pid);
            printf("\n");
        }
    }

}



void viewJobs(){
    int index  = 0;
    printf("\n");
    printf("\t==Active Processes==\n\n");
    printf("| %s  | %s | %s |\n",  "   PID   ", "   Job ID   ", "   Command   ");
    for (index = 0; index < numJobs; index++){
        if(kill(jobs[index].pid, 0) == 0){
            printf("|  [%d] | %d | %s |\n", jobs[index].pid, jobs[index].id, jobs[index].cmd);
        }
    }
}

void createPipe(){
    char *part = strtok(current, "|\0");
    char *command = part;
    part = strtok(NULL, "\0");
    int pipefd[2];
    pid_t pid, pid2;
    pipe(pipefd);
    pid = fork();
    if (pid == 0){
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

int sPath(char *action){
    char *pt = strtok(action, "=");
    char *path = strtok(NULL, "\0");
    if ((setenv(pt, path, 1)) == -1){
        printf("%s wasn't set correctly.\n", pt);
    }
    return 1;
}

void performAction(){
    char *command;
    char *args[20];
    for (int i = 0; i < 20; i++){ args[i] = NULL; }
    int numArgs = 0;
    char *input = strdup(current);
    command = strtok(current, " ");
    while (command != NULL){
        args[numArgs++] = command;
        command = strtok(NULL, " ");
    }

    if (strcmp("cd", args[0]) == 0){
        changeDirectory(args[1]);
    }
    else if (strcmp(args[0], "set") == 0){
        sPath(args[1]);
    }
    else if (strchr(input, '|') != NULL){
        createPipe();
    }
    else if (strchr(input, '&') != NULL){
        viewProcess();
    }
    else if (strstr(input, "kill") != NULL){
        killProcess(args);
    }
    else if (strcmp(args[0], "jobs") == 0){
        viewJobs();
    }
    else if (strchr(input, '<') != NULL){
        in(args);
    }
    else if (strchr(input, '>') != NULL){
        out(args);
    }
    else{
        ex(args);
    }
}


int main(int argc, char *argv[]){
    printf("EECS678-QUASH\n\n");
    char *action, prompt[128];
    env = getenv("USER");
    dir = getcwd(NULL, 1024);
    numJobs = 0;

    while (1){
        snprintf(prompt, sizeof(prompt), "%s:%s> ", env, dir);
        action = readline(prompt);
        action = clearWhitespace(action);
        if (strcmp("exit", action) != 0 && strcmp("quit", action) != 0){
            if (strlen(action) > 1){
                action = clearWhitespace(action);
                current = action;
                performAction();
            }
        }
        else{
            break;
        }
        free(action);
    }

    return 0;
}
