#include <stdio.h>
#include <string.h> // to be only used for strtok()
int main(){
    while(1){
        char user_input[100];
        char deli[] = " "; 
        printf("User@system:~$");
        fgets(user_input, sizeof(user_input), stdin);
        
        char * tokenptr = strtok(user_input, deli);
        int i = 0;
        while(tokenptr != NULL){
            printf("arg[%d] = %s\n", i, tokenptr);
            tokenptr = strtok(NULL, deli);
            i++;
        }
    }
    return 0;
}   