#include <stdio.h>
#include <stdlib.h>
#include <string.h>             // to be only used for strtok()
#include <unistd.h>             // used for system calls POSIX 
#include <sys/wait.h>
#include <signal.h>

#include "parser.h"
#include "tokenizer.h"
#include "command.h"
#include "executor.h"
#include "builtins.h"
#include "termios.h"

// will store the original terminal state, before i go RAW MODE 
struct termios orignal_state;

struct termios raw_state;

struct cmd_history
{
    char *h_cmds;
    struct cmd_history *next;
    struct cmd_history *previous;
};

// Job table 
Jobs job_tble[20];
int job_number = 1;

// history command head
struct cmd_history *head = NULL;
struct cmd_history *tail = NULL;

void header(){  

    printf(
    "       o           oooo     oooo      ooooo  oooo      oooo   oooo      ooooo      ooooo  oooo       \n"
    "      888           8888o   888        888    88        8888o  88        888         888  88         \n"
    "     8  88          88 888o8 88        888    88        88 888o88        888           888           \n"
    "    8oooo88         88  888  88        888    88        88   8888        888           888       \n"
    "   88     88        88   88  88        888    88        88    888        888         888  88         \n"
    "  88       88       88       88        888    88        88     88        888        888    88        \n"
    " o88o    o888o     o88o     o88o       o888oo88o       o88o    o88o     o888o     o88o    o888o      \n"
    "                                                                                                  \n"
    );

    printf("\nThis project is being actively developed as a learning and building exercise."
            "\nSome components may not be stable yet."
            "\nIf you run into issues or have suggestions, reporting them would be helpful by raising issues on our GITHUB repo.\n");

    printf("\n\x1b[31mAMUIX is an another shell in this open source world. It is used to study the shell development and operating system working. Refer to this repo 'https://github.com/Cryogenicboom/User-Simulated-Virtual-OS' where we are simulating the operating system.\x1b[0m\n\n");
}

void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orignal_state);
    // free the whole list
    if(head != NULL)
    {
        struct cmd_history *next_ptr = head->next;
        struct cmd_history *curnt_ptr = head;
        while(curnt_ptr != NULL)
        {
            free(curnt_ptr);
            curnt_ptr = next_ptr;
            if(curnt_ptr != NULL)
            {
                next_ptr = curnt_ptr->next;
            }
        }
        free(next_ptr);
        free(curnt_ptr);
    }

}

int arrow_keys(char single_char, char user_input[100], struct cmd_history *tail, struct cmd_history *head, int i, struct cmd_history **navptr)
{   
    single_char = getchar();

    if(single_char == 91)
    {   

        single_char = getchar();
        
        for(int j = 0; j < i; j++)
        {
            printf("\b \b");
        }

        if(single_char == 65)
            {   
                if(*navptr == NULL) 
                {
                    return 0;
                }
                printf("%s", (*navptr)->h_cmds);
                strcpy(user_input, (*navptr)->h_cmds);
                int len = strlen(user_input);

                // *navptr = (*navptr)->previous;

                if((*navptr)->previous != NULL)
                {
                    *navptr = (*navptr)->previous;
                }

                return len;
            }
        else if(single_char == 66)
        {   
            if(*navptr == NULL)
            {
                return 0;
            }
            if((*navptr)->next != NULL)
            {
                *navptr = (*navptr)->next;
                printf("%s", (*navptr)->h_cmds);
                strcpy(user_input, (*navptr)->h_cmds);
                return strlen(user_input);
            }
            else 
            {   
                user_input[0] = '\0';
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    return -1;              // fail
}

int main()
{
    header();
    signal(SIGTTOU, SIG_IGN);
    signal(SIGINT, SIG_IGN);                    // ignore the shell crtl+c interrupt

    // struct termios orignal_state;
    tcgetattr(STDIN_FILENO, &orignal_state);

    // whenever exit is called switch to og terminal state
    atexit(restore_terminal);

    // copy the original state into another struct and then use it to modify for raw.
    raw_state = orignal_state;

    raw_state.c_lflag = raw_state.c_lflag & (~ICANON);
    raw_state.c_lflag = raw_state.c_lflag & (~ECHO);

    // cooked ---> raw mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_state);

    struct cmd_history *navptr = tail;                // pointer to navigate the history array

    // initial status of all array slots  = 0
    for(int i = 0; i < 20; i++)
    {
        job_tble[i].status = 0;
    }

    // fetch username and system name
    char *username;
    if(getlogin() != NULL)
    {
        username = strdup(getlogin());              // getlogin returns pointer which will be hold by username pointer
    }
    else if(getlogin() == NULL)
    {
        username = strdup("Guest");
    }
    
    char sys_name[_SC_HOST_NAME_MAX];
    gethostname(sys_name, sizeof(sys_name));

    char sound[4];

    while(1)
    {
        char user_input[100];
        char single_char;
        char *tok_cmds[300];                             //these commands are tokenized only
        char *parsed_cmds[300] = {NULL};                // these commands are parsed matlab, [ERROR 4 in diary]
        pid_t bg_pid;
        int status;

        // Background process check and terminates
        while((bg_pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            printf("\n\033[33m[done] exit pid = %d\033[0m", bg_pid);
            for(int i = 0; i <20; i++)
            {
                if(job_tble[i].pid == bg_pid)
                {
                    job_tble[i].status = 0;
                    job_number--;
                    printf("job_num: %d\n", job_number);
                    break;
                }
            } 
        }
        
        if(dog_flag == 1)
        {
            strcpy(sound, "ON");
        }
        else if(dog_flag == 0)
        {
            strcpy(sound, "OFF");
        }


        char pwd[100];
        char prompt[512];
        if(getcwd(pwd, sizeof(pwd)) != NULL)
        {
            snprintf(prompt, sizeof(prompt), "%s@%s:%s $", username, sys_name, pwd);
            printf("\n\x1b[38;5;250mJobs: %d, Sound: %s\e[0m", job_number, sound);
            printf("\n\x1b[32m%s\x1b[0m", prompt);
        }

        // RAW MODE ======================================================================
        int i = 0;
        // int prompt_len = strlen(prompt);
        single_char = getchar();

        while(single_char != '\n')
        {
            // printf("(%d)", single_char);

            if(single_char == 27)
            {   
                int temp = arrow_keys(single_char, user_input, tail, head, i, &navptr);
                if(temp >= 0)               
                {
                    i = temp;
                }
            }
            else if(single_char == 127 && i > 0)
            {
                printf("\b \b");
                i--;
            }
            else
            {   
                if(single_char != 127)
                {
                    putchar(single_char);
                    user_input[i] = single_char;
                    
                    fflush(stdout);                      // Notes in diary( 14 march).
                    i++ ;
                }
            }
            single_char = getchar();

        }
        putchar('\n');
        user_input[i] = '\0';

        if(dog_flag == 1)
        {
            system(system_arg);
        }

        if(strlen(user_input) != 0)
        {
            // "if block "only runs at start, when tail = head
            if(tail == NULL)
            {
                tail = (struct cmd_history *) malloc(sizeof(struct cmd_history));   // first node 
                tail->h_cmds = strdup(user_input);
                tail->previous = NULL;
                head = tail;                      // head points to first node 
                tail->next = NULL;
            }
            else
            {   
                // next node create || Check march 22 diary entry for info
                struct cmd_history *temp = NULL;
                temp = (struct cmd_history *) malloc(sizeof(struct cmd_history));
                temp->previous = tail;                  // store the prev. node address to new node's prev ptr
                tail->next = temp;                      // old node store the new node address
                tail = temp;                            // move tail to new node
                tail->h_cmds = strdup(user_input);      // store the new command in new node
                tail->next = NULL;   


            }
            navptr = tail;
        }

        // add spaces before and after pipe 
        int buffer_idx = 0;
        char buffer[300];
        for(int user_idx =0; user_input[user_idx] != '\0' ; user_idx++)
        {
            if(user_input[user_idx] == '|')
            {
                
                buffer[buffer_idx] = ' ' ;
                buffer[buffer_idx + 1] = '|';
                buffer[buffer_idx + 2] = ' ';
                buffer_idx += 3;

            }
            else
            {
                buffer[buffer_idx] = user_input[user_idx];
                buffer_idx += 1;
            }
        }

        buffer[buffer_idx] = '\0';
        strcpy(user_input, buffer);


        // =================================== TOKENIZE ==================================

        tokenize(user_input, tok_cmds);
        
        // =================================== Parsing ======================================

        Command cmd;
        cmd.count = 0;

        parser_for_quotes(tok_cmds, parsed_cmds);
        parse_struct(parsed_cmds, &cmd);

        // // DEBUG 
        // printf("\nStruct Debug \n");
        // for(int i = 0; i < cmd.count; i++){
        //     printf("simpleCommand %d: ", i);
        //     for(int j = 0; cmd.simpleCommands[i].argv[j] != NULL; j++){
        //         printf("[%s] ", cmd.simpleCommands[i].argv[j]);
        //     }
        //     printf("\n");
        // }
        // DEBUG END

        if(parsed_cmds[0] == NULL)
        {
            for(int i = 0; tok_cmds[i] != NULL; i++)                // free tokens
            {
                free(tok_cmds[i]);
            }

            for(int i = 0; parsed_cmds[i] != NULL; i++)             // free parsed commands
            {
                free(parsed_cmds[i]);
            }

            continue;
        }

        // // debug
        // printf("\n      DEBUG: parsed_cmds[0] = %p\n\n", parsed_cmds[0]); //check for seg fault 
        // // debug end

        // ========================================BUILT IN CMDS: ========================================
        if(built_ins(parsed_cmds) != 0)             
        {
            // ===============External Cmds: ===============
            execute_command(&cmd, cmd.count);
        }
 
        // free the strdup heap memory allocation
        free_command_mem(&cmd);                                 // IO files from command strcut
        
        for(int i = 0; tok_cmds[i] != NULL; i++)                // free tokens
        {
            free(tok_cmds[i]);
        }

        for(int i = 0; parsed_cmds[i] != NULL; i++)             // free parsed commands
        {
            free(parsed_cmds[i]);
        }

    }
    free(username);
    return 0;
}   