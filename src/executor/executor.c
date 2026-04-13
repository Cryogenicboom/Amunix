#include <stdio.h>
#include <stdlib.h>
#include <string.h>             // to be only used for strtok()
#include <unistd.h>             // used for system calls POSIX 
#include <sys/wait.h>
#include "command.h"

void execute_command(Command *cmd, int cmd_count)
{
    int fdin = 0;   // intial command is read from keyboard "stdin". fdin will be updated to track which input should next command read from.

    for(int i = 0; i < cmd_count; i ++)
        {
            int fd[2];

            // create pipe connection only when needed, like at last command no pipe is needed. 
            if(i < cmd_count - 1)
            {
                pipe(fd);
            }

            pid_t pid = fork();
            

            if(pid == 0)
            {
                dup2(fdin, 0);
                
                if(i < cmd_count - 1)
                {
                    dup2(fd[1], 1);
                    close(fd[0]);
                }
                
                execvp(cmd->simpleCommands[i].argv[0], cmd->simpleCommands[i].argv);
                perror("execvp failed");
                exit(1);
            }
            
            else if (pid > 0)
            {
                if(i < cmd_count-1)
                {
                    close(fd[1]);
                    fdin = fd[0];
                }
            }
            else
            {
                printf("error, fork failed");

            }
        }
        
        if(fdin != 0)
        {
            close(fdin);
        }

        for(int i =0; i < cmd_count; i ++)
        {
            wait(NULL);
        }
}