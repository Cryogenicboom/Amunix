#include <stdio.h>
#include <stdlib.h>
#include <string.h>             // to be only used for strtok()
#include <unistd.h>             // used for system calls POSIX 
#include <sys/wait.h>
#include "command.h"

void execute_command(Command *cmd, int cmd_count)
{
    for(int i = 0; i < cmd_count; i ++)
        {
            pid_t pid = fork();
            if(pid == 0)
            {
                execvp(cmd->simpleCommands[i].argv[0], cmd->simpleCommands[i].argv);
                perror("execvp failed");
                exit(1);
            }
            
            else if (pid < 0)
            {
                printf("error, fork failed");
            }
        }
        for(int i =0; i < cmd_count; i ++)
        {
            wait(NULL);
        }
}