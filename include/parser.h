#ifndef PARSER_H
#define PARSER_H
#include "command.h"

void parser_for_quotes(char * cmds[], char * parsed_cmds[]);
void parse_struct(char *tokens[], Command *cmd);


#endif