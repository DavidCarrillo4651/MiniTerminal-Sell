#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "builtin.h"
#include "strextra.h"
#include "command.h"
#include "tests/syscall_mock.h"

bool builtin_is_internal(scommand cmd){
    assert(cmd!=NULL);
    char *command = scommand_front(cmd);
    return !strcmp(command, "cd")   || 
            !strcmp(command, "help") || 
            !strcmp(command, "exit");
            //strcmp devuelve 0 si las dos cadenas ingresadas son iguales.
}

bool builtin_alone(pipeline p){
    assert(p != NULL);
    return (pipeline_length(p) == 1 && 
            builtin_is_internal(pipeline_front(p)));
}

void builtin_run(scommand cmd){
    assert(builtin_is_internal(cmd));
	char *command = scommand_front(cmd);
    if(!strcmp(command, "exit")){
       exit(0);
    
    } else if(!strcmp(command, "cd")){
        
        scommand_pop_front(cmd);
        if(!scommand_is_empty(cmd)){
        int chdir_stat = chdir(scommand_front(cmd));
        if(chdir_stat != 0){

            perror("Error al entrar al directorion\n");
            }
        }
    } else if (!strcmp(command, "help")){
        printf("\033[1;32m  __  __       ____            _         ___   ___ ___  ____  \n"
	"\033[1;32m |  \\/  |     |  _ \\          | |       |__ \\ / _ \\__ \\|___ \\ \n"
	"\033[1;32m | \\  / |_   _| |_) | __ _ ___| |__ ______ ) | | | | ) | __) |\n"
	"\033[1;32m | |\\/| | | | |  _ < / _` / __| '_ \\______/ /| | | |/ / |__ < \n"
	"\033[1;32m | |  | | |_| | |_) | (_| \\__ \\ | | |    / /_| |_| / /_ ___) |\n"
	"\033[1;32m |_|  |_|\\__, |____/ \\__,_|___/_| |_|   |____|\\___/____|____/ \n"
	"\033[1;32m          __/ |                                               \n"
	"\033[1;32m         |___/                                                \n");

	printf(
		"\033[1;33m                                     __          \n"
		"\033[1;33m  _  _     _  _     _  _     _  _   |  |--.--.--.\n"
		"\033[1;33m (_)(_)   (_)(_)   (_)(_)   (_)(_)  |  _  |  |  |\n"
		"\033[1;33m (_)(_)   (_)(_)   (_)(_)   (_)(_)  |_____|___  |\n"
		"\033[1;33m                                          |_____|\n\n"

		"\033[1;37mDiogenes Puig\n"
		"\033[1;37mFelipe Carrizo\n"
		"\033[1;37mDavid Carrillo\n"
		"\033[1;37mLeandro Tallarico\n\n\n"


		"\033[1;33mA continuación proveemos una breve descripción de lo que hace este shell:\n\n"

		"\033[1;32m-- help : Imprime este mismo mensaje de ayuda\n"
		"\033[1;32m-- exit : Termina este bash\n"
		"\033[1;32m-- cd 'NOMBRE_DIRECTORIO': Cambia el directorio a 'NOMBRE_DIRECTORIO'\n"
		"\033[1;32m-- ls : Imprime en pantalla el contenido del directorio actual\n"
		"\033[1;32m-- wc 'NOMBRE_FICHERO': Imprime en pantalla la cantidad de lineas, bytes y nombre 'NOMBRE_FICHERO'\n"
		"\033[1;32m-- xeyes : Pone en pantalla unos ojos que siguen al cursor\n\n");

    }
}
