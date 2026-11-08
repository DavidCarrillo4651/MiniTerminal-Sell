#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

static void show_prompt(void) {
    printf("\033[0;32m");//seteado el print en verde.
    printf ("%s$ ", getenv("PWD"));
    printf ("\033[0m");//seteado el print en negro.
    printf ("mybash> ");
    fflush (stdout);
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input;
    bool quit = false;

    input = parser_new(stdin);
    while (!quit) {
        show_prompt();
        pipe = parse_pipeline(input);

        /* Hay que salir luego de ejecutar? */
        quit = parser_at_eof(input);
        //guardo en quit si se llego al final del arhivo
        if(pipe != NULL){
            execute_pipeline(pipe);
            pipe = pipeline_destroy(pipe);
        }
    }
    //salto de linea pa que sea bonito
    printf("\n");
    parser_destroy(input); input = NULL;
    return EXIT_SUCCESS;
}

