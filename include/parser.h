#ifndef PARSER_H
#define PARSER_H
#include "command.h"

void parser_for_quotes(char * cmds[], char * parsed_cmds[]);
void parsing_by_special_char(char * parsed_cmds[], char * commands[10][50], int *cmd_count, Command *cmd);


#endif