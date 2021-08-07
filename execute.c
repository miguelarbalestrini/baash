#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "execute.h"
#include "tests/syscall_mock.h"
/* #include "mili.h" en mi corazoncito */

static void load_array(char** sc_array, pipeline apipe) {
    char* cmd = NULL;
    unsigned int k = 0;
    do {
    	cmd = bstr2cstr(scommand_front(pipeline_front(apipe)), '\0');
    	sc_array[k] = cmd;
        scommand_pop_front(pipeline_front(apipe));
        k++;
    } while (!scommand_is_empty(pipeline_front(apipe)));
    sc_array[k] = NULL;
}

void execute_pipeline(pipeline apipe) {
    if (!pipeline_length(apipe))
        return;
    const unsigned int pipeLen = pipeline_length(apipe);
	const unsigned int numPipes = pipeLen-1;
    int pipefds[2*pipeLen];
    unsigned int j = 0; unsigned int k = 0;
    pid_t pid[pipeLen];
    int filefd;

    /* Comprueba comandos internos */
    if (builtin_index(apipe) != BUILTIN_UNKNOWN)
        builtin_run(apipe);
    else {
    	/* Crea todos los pipes necesarios */
    	for(unsigned int i = 0; i < numPipes; i++) {
        	if(pipe(pipefds + i*2) < 0) {
            	perror("Error al crear pipe");
            	exit(EXIT_FAILURE);
        	}
    	}
    	/* Se recorre de a un scommand y se lo va quitando del pipeline */
    	while (!pipeline_is_empty(apipe)) {
            unsigned int scLen = scommand_length(pipeline_front(apipe));
            char** sc_array = (char**)calloc(scLen+1, sizeof(char*));
            /* Carga un scommand en un arreglo cuyo último elemento es NULL */
            load_array(sc_array, apipe);

            if ((pid[k] = fork()) < 0) { //Si falla al crear proceso hijo
                perror("Error al crear proceso hijo");
                exit(EXIT_FAILURE);
            }
        	else if(pid[k] == 0) { //Proceso hijo
                /* Si el comando tiene redirección de entrada */
                if (scommand_get_redir_in(pipeline_front(apipe))) {
                    filefd = open(bstr2cstr(scommand_get_redir_in(pipeline_front(apipe)), '\0'), O_RDONLY,0640);
                    dup2(filefd, 0);
                    close(filefd);
                }
                else {
                    /* Si no es el primer comando */
                    if (pipeline_length(apipe) != pipeLen) {
                        if(dup2(pipefds[j-2], 0) < 0) {
                            perror("Error al duplicar stdin");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                /* Si el comando tiene redirección de salida */
                if (scommand_get_redir_out(pipeline_front(apipe))) {
                    filefd = open(bstr2cstr(scommand_get_redir_out(pipeline_front(apipe)), '\0'), O_WRONLY|O_CREAT|O_TRUNC, 0640);
                    dup2(filefd, 1);
                    close(filefd);
                }
                else {
                    /* Si no es el último comando */
                    if (pipeline_length(apipe) > 1) {
                        if(dup2(pipefds[j+1], 1) < 0) {
                            perror("Error al duplicar stdout");
                            exit(EXIT_FAILURE);
                        }
                    }
                }

            	/* Cerrar todos los pipes en el proceso hijo */
            	for(unsigned int i = 0; i < 2*numPipes; i++)
                	close(pipefds[i]);
            	/* Ejecuta el comando */
            	if( execvp(sc_array[0], sc_array) < 0 ) {
                	perror(sc_array[0]);
                	exit(EXIT_FAILURE);
            	}
                close(filefd);
        	}
            else { //Proceso padre
                /* Se quita el scommand actual del pipeline */
                pipeline_pop_front(apipe);
                j+=2; k++;
            }
            /* Liberación de memoria */
            for (unsigned int i = 0; i < pipeLen; i++)
            	bcstrfree(sc_array[i]);
            free(sc_array);
    	}
        /*Cerrar todos los pipes en el proceso padre */
     	for(unsigned int i = 0; i < 2*numPipes; i++)
        	close(pipefds[i]);
        /* Espera de los procesos hijos */
     	if (pipeline_get_wait(apipe)) {
        	for(unsigned int i = 0; i < pipeLen; i++)
                if (waitpid(pid[i], NULL, 0) < 0) {
                    perror("Esperando a cada hijo");
                    exit(1);
                }
    	}
	}
}
