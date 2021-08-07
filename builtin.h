#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>
#include "command.h"


typedef enum {
    BUILTIN_EXIT = 0,
    BUILTIN_CHDIR = 1,
    BUILTIN_UNKNOWN = -1
} cmd_id;

cmd_id builtin_index (const pipeline pipe);

bool builtin_is_exit (const pipeline pipe);
/* Este caso partcular es util para cortar el ciclo principal */
void builtin_run (const pipeline pipe);

#endif
