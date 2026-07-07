// returns 0 if builtins used else returns 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>             // to be only used for strtok()
#include <unistd.h>             // used for system calls POSIX 
#include <sys/wait.h>
#include <pwd.h>
#include "command.h"
#include <libgen.h>
#include <termio.h>
#include <signal.h>

#define PATH_MAX 4095

char path[PATH_MAX];
char system_arg[PATH_MAX+50];
int dog_flag = 0;               // 0: off, 1: on
pid_t fg_gid;
// char buf[PATH_MAX];

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

    else if(strcmp(parsed_cmds[0], "dog") == 0)
    {

        char *buf;
        readlink("/proc/self/exe", path, PATH_MAX);
        buf = dirname(path);

        char * dog_in = "/Media/Bark.mp3";
        char * dog_out = "/Media/yelp.mp3";

        if(strcmp(parsed_cmds[1], "-e") == 0)
        {
            dog_flag = 1;
            strcat(buf, dog_in);
            snprintf(system_arg, sizeof(system_arg), "mpg123 -q %s > /dev/null 2>&1 &", buf);
            system(system_arg);
            return 0;
        }
        else if(strcmp(parsed_cmds[1], "-d") ==0)
        {
            dog_flag = 0;
            strcat(buf, dog_out);
            snprintf(system_arg, sizeof(system_arg), "mpg123 -q %s > /dev/null 2>&1 &", buf);
            system(system_arg);
            return 0;
        }
        else
        {
            printf("\nError No arguments defined. Type 'help' for help which do not works\n");
            return 1;
        }
    }

    else if(strcmp(parsed_cmds[0], "fg") == 0)
    {
        int status;
        if(parsed_cmds[1] != NULL)
        {
            int target = atoi(parsed_cmds[1]);
            for(int i = 0; i < 20; i++)
            {
                if(job_tble[i].job_id == target && job_tble[i].status == 1)
                {
                    fg_gid = job_tble[i].gid;
                    job_tble[i].status = 0;
                    break;
                }
            }

            tcsetpgrp(STDIN_FILENO, fg_gid);
            waitpid(fg_gid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpgrp());
        }
        return 0;
    }
    
    else if(strcmp(parsed_cmds[0], "bg") == 0)
    {
        if(parsed_cmds[1] != NULL)
        {
            int target = atoi(parsed_cmds[1]);
            for(int i = 0; i < 20; i++)
            {
                if(job_tble[i].job_id == target && job_tble[i].status == 1)
                {
                    kill(-job_tble[i].gid, SIGCONT);
                    break;
                }
            }
        }
        return 0;
    }
    else
    {
        return 1;
    }
}
