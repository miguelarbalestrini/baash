#include <stdlib.h>
#include <stdio.h>
#include "command.h"
#include "parser.h"
#include "execute.h"
#include "unistd.h"
#include <assert.h>
#include "builtin.h"
#define _XOPEN_SOURCE_EXTENDED 1

struct{
    char host[1024];
    char dir[1024];
    char* usr;
    char* str$; 
    char* str1;
    char str2[1024];
    char* str3;
}prompt;

int gethostname(char *name, size_t len);

static void init_prompt(void){

    getcwd(prompt.dir, sizeof(prompt.dir));
   	gethostname(prompt.host, sizeof(prompt.host));
    
    prompt.usr = (getlogin());
    prompt.str1 = "@";
    prompt.str3 = ":~";
    prompt.str$ = "$ ";
    
    strcpy(prompt.str2, prompt.usr);
    strcat(prompt.str2, prompt.str1);
}

static void print_prompt(char* host, char* dir, char* str$, char* str2, char* str3){
        printf("\033[1;36m");
        printf("%s%s%s", str2, host, str3);
        printf("\033[0m");
        printf("\033[1;35m");
        printf("%s", dir);   
        printf("\033[0m");
        printf("%s", str$);
}

int main(void){
   
    Parser prs = NULL;
    pipeline pipe = NULL;  
    system("clear");
    init_prompt();
    
    while(1){
        
        getcwd(prompt.dir, sizeof(prompt.dir)); 
        print_prompt(prompt.host, prompt.dir,  prompt.str$, prompt.str2, prompt.str3);
        
        prs = parser_new(stdin);
        pipe = parse_pipeline(prs);
        
        if (pipe!=NULL){
            execute_pipeline(pipe);

        }else{
            stderr;
        }

        if (parser_at_eof(prs)){
            printf("\n");
            exit(EXIT_SUCCESS);
        }
        
        prs = parser_destroy(prs);
        fflush(stdin);
    }
    return 0;
}