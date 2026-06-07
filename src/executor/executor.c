#include <stdio.h>
#include <stdlib.h>
#include <string.h>             // to be only used for strtok()
#include <unistd.h>             // used for system calls POSIX 
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "command.h"
#include <termio.h>
#include "executor.h"

void execute_command(Command *cmd, int cmd_count)
{

    // int fdin = 0;   // intial command is read from keyboard "stdin". fdin will be updated to track which input should next command read from.
    int fdin;          // fd where current command read from


    if(cmd->inputfile != NULL)
    {
        fdin = open(cmd->inputfile, O_RDONLY);
    }
    else
    {
        fdin = dup(0);
    }

    pid_t parent_gid = getpgrp();

    for(int i = 0; i < cmd_count; i ++)
        {
            int fd[2];

            // create pipe connection only when needed, like at last command no pipe is needed. 
            if(i < cmd_count - 1)
            {
                pipe(fd);
            }

            pid_t pid = fork();             // each command is a new process with their own file desc. table

            if(pid == 0)                    // fork return 0 if child is successfully created
            {   
                setpgid(pid, 0);            // change child's process group

                dup2(fdin, 0);
                close(fdin);

                int fdout;

                if(i == cmd_count - 1)      // last command 
                {
                    if(cmd->outputfile != NULL)
                    {
                        fdout = open(cmd->outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    }
                    else
                    {
                        fdout = dup(1);
                    }
                    
                }
                else
                {
                    fdout = fd[1];
                    close(fd[0]);    // child doesn’t read from this pipe
                }

                dup2(fdout, 1);
                close(fdout);
                execvp(cmd->simpleCommands[i].argv[0], cmd->simpleCommands[i].argv);
                perror("execvp failed");
                _exit(1);                                        // _exit() prevents child changing mode
            }
            
            else if (pid > 0)
            {   
                setpgid(pid, pid);                              // parents changes the child's group

                if(cmd->bg_status != 1)                         // Do not make child foreground if it is set to be bg (&)
                {
                    tcsetpgrp(STDIN_FILENO, pid);               // parents makes child foreground groupm
                }

                close(fdin);
                if(i < cmd_count-1)
                {
                    close(fd[1]);       // parent do not write
                    fdin = fd[0];       // next command reads this
                }
            }
            else
            {
                perror("error, fork failed");

            }
        }
        
        if(fdin != 0)
        {
            close(fdin);
        }
        if (cmd->bg_status != 1)
        {
            for(int i =0; i < cmd_count; i ++)
            {
                wait(NULL);
            }
            tcsetpgrp(STDIN_FILENO, parent_gid);
        }
        
} 