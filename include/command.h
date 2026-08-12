#ifndef COMMAND_H
#define COMMAND_H

#include "sys/types.h"

typedef struct{
    int argc;
    char *argv[50];
} SimpleCommand;

typedef struct{
    int count;
    SimpleCommand simpleCommands [10];  // like we did before commands[10][50]
    char *inputfile;
    char *outputfile;
    int bg_status;                      // & : 0 -> false, 1 -> true
} Command ;

void free_command_mem(Command *cmd);

typedef struct 
{
    pid_t pid; 
    pid_t gid;
    int status;                             // 0 -> Empty strcut process complete, 1 -> occupied / running , -1 -> error and crash
    int job_id;
}Jobs;

extern Jobs job_tble[20];
extern int job_number;

extern int active_jobs;

#endif