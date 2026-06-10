// returns 0 if builtins used else returns 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>             // to be only used for strtok()
#include <unistd.h>             // used for system calls POSIX 
#include <sys/wait.h>
#include <pwd.h>
#include "command.h"

int built_ins(char *parsed_cmds[])
{

    if(strcmp(parsed_cmds[0], "dbd") == 0)
    {
        printf("        DEBUG: [%s, %s, %s]\n", parsed_cmds[0], parsed_cmds[1], parsed_cmds[2]);
        if(chdir(parsed_cmds[1]) == -1)         // chdir us used to dirbadlo
        {
            perror("command : directory badlo failed");
        }
        return 0;
    }
    else if (strcmp(parsed_cmds[0], "bahar") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(parsed_cmds[0], "whoru") == 0)
    {
        
        printf("\ni am a shell who helps you interact with your Operating system and helps with using your system. Name is AMUNIX, idk what it means but thanks for asking!");
        return 0;
    }
    else if(strcmp(parsed_cmds[0], "Hello") == 0)
    {
        struct passwd *pw;
        __uid_t uid;

        uid = getuid();
        pw = getpwuid(uid);

        if(pw)
        {
            printf("Hello %s, How it is going, how may i help you?\n", pw->pw_name);
        }
        else
        {
            perror("getpwuid");
            return 1;
        }
        return 0;
    }

    else if(strcmp(parsed_cmds[0], "jobs") == 0)
    {
        for(int i = 0; i < 20; i++)
        {
            if(job_tble[i].status == 1)
            {
                printf("\n%-10d %-10d %-10d",job_tble[i].job_id, job_tble[i].pid, job_tble[i].gid);
            }
        }
        return 0;
    }
    else
    {
        return 1;
    }
}
