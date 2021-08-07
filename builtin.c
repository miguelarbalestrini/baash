#include "builtin.h"
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tests/syscall_mock.h"

cmd_id builtin_index (const pipeline pipe){
    assert(!pipeline_is_empty(pipe));
    char * aux = bstr2cstr(scommand_front(pipeline_front(pipe)),' ');
    char * cede = "cd";
    char * exit = "exit";

    if(strcmp(cede, aux) == 0){
        return BUILTIN_CHDIR;
    }
    else if (strcmp(exit, aux) == 0){
        return BUILTIN_EXIT;
    }
    else{
        return BUILTIN_UNKNOWN;
    }
}

bool builtin_is_exit (const pipeline pipe){
    assert(!pipeline_is_empty(pipe));

    char *exit = "exit";
    char *aux = bstr2cstr(scommand_front(pipeline_front(pipe)),' ');
        if(strcmp(aux, exit) == 0){
            return true;
        }
        else {
            return false;
        }
}

void builtin_run (const pipeline pipe){
    assert(!pipeline_is_empty(pipe));

    if (builtin_index(pipe) == BUILTIN_EXIT){
        exit(EXIT_SUCCESS);
    }
    else if(builtin_index(pipe) == BUILTIN_CHDIR){
        scommand cd = pipeline_front(pipe);
        scommand_pop_front(cd);
        bstring path;
        if(scommand_is_empty(cd)){
            //printf("Me voy al home");
            chdir(getenv("home"));
        }
        else{
            //printf("Me voy al path");
            const char * the_path;
            path = (bstring) scommand_front(cd);
            the_path = (const char *) path->data;
            chdir(the_path);
        }
    }
}
