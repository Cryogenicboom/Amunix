#include <stdlib.h>
#include "command.h"

void free_command_mem(Command *cmd)             // free strdup used by IO files during parsing.
{   
    if(cmd->inputfile  != NULL)
    {
        free(cmd->inputfile);
    }
   if(cmd->outputfile  != NULL)
    {
        free(cmd->outputfile);
    }
    return;
}