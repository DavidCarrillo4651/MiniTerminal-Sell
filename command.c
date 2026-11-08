#include <stdbool.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "strextra.h"
#include "command.h"

//----- Implementacion scommand (12) -----
struct scommand_s{
    GQueue *command;   // commanda de string(comandos escritos por stdin)
    char *out;      // Redireccionamiento out
    char *in;       // Redireccionamiento in
};

scommand scommand_new(void) {
    scommand n;
    n = malloc(sizeof(struct scommand_s));
    n->command = g_queue_new();  // COMENTARIO: g_queue_new() es de tipo Queue * 
    n->out = NULL;
    n->in = NULL;

    assert(n!= NULL && scommand_is_empty (n) && 
            scommand_get_redir_in (n) == NULL && 
            scommand_get_redir_out (n) == NULL);
    return n;
}

// Funcion auxiliar para llamar en g_queue_free_full()
static void free_function(gpointer data) {
    if (data != NULL) {
        g_free(data); data = NULL; // Liberar la memoria de la cadena de caracteres
    }
}
scommand scommand_destroy(scommand self){
    assert(self!=NULL);
    free(self->out); self->out = NULL;
    free(self->in); self->in = NULL;

    if (self->command!=NULL){
        g_queue_free_full(self->command, free_function); 
        self->command = NULL;
    }
    free(self); self = NULL;
    assert(self==NULL);
    return self;
}


void scommand_push_back(scommand self, char * argument){
    assert(self!=NULL && argument!=NULL);
    g_queue_push_tail(self->command,argument);
    assert(!scommand_is_empty(self));
}


void scommand_pop_front(scommand self) {
    assert(self!=NULL && !scommand_is_empty(self));

    // Se quita el primer elemento y se lo asigna a poped_element
    char * popped_element = g_queue_pop_head(self->command); 
    // Se libera dicho elemento para evitar memory leaks
    free(popped_element); popped_element = NULL; 
}


void scommand_set_redir_in(scommand self, char * filename){
    assert(self!=NULL);
    // libero memoria de ser necesario
    if(self->in!=NULL){
    	free(self->in);
    }
    self->in = filename;
}

void scommand_set_redir_out(scommand self, char * filename){
    assert(self!=NULL);
    // libero memoria de ser necesario
    if(self->out!=NULL){
    	free(self->out);
    }
    self->out = filename;
}


bool scommand_is_empty(const scommand self){
    assert(self != NULL);
    return g_queue_is_empty(self->command);
}


unsigned int scommand_length(const scommand self){
    assert(self!=NULL);
    unsigned int size = g_queue_get_length(self->command);
    assert((g_queue_get_length(self->command)==0) == scommand_is_empty(self));
    return size;
}


char * scommand_front(const scommand self){
    assert(self!=NULL && !scommand_is_empty(self));

    // Se devuelve un puntero al primer elemento, el elemento sigue formando parte de la cola
    char * result = g_queue_peek_head(self->command);
    assert(result!=NULL);
    return result;
}


char * scommand_get_redir_in(const scommand self){
    assert(self!=NULL);
    return self->in;
}

char * scommand_get_redir_out(const scommand self){
    assert(self!=NULL);
    return self->out;
}


char *scommand_to_string(const scommand self) {
    assert(self != NULL);
    
    // Inicialmente, solo un carácter nulo (caso: cadena vacía)
    char *str_scmd = (char *)malloc(1); 
    str_scmd[0] = '\0';

    // Tiene comando?
    if (!g_queue_is_empty(self->command)) {

        guint length = g_queue_get_length(self->command);

        // Se itera sobre el comando y sus argumentos, agregandolos a la cadena
        for (guint i = 0; i < length; i++) {
            char *element = (char *)g_queue_peek_nth(self->command, i);
            char *new_str = strmerge(str_scmd, element);

            // Se libera la memoria previamente asignada
            free(str_scmd);  
            str_scmd = new_str;

            // Si no es el último elemento, se agrega un espacio
            if (i < length - 1) {
                char *whit_space = strmerge(str_scmd, " ");
                free(str_scmd);
                str_scmd = whit_space;
            }
        }
    }

    // Tiene redireccionamiento out?
    if (self->out != NULL) {
        char *out_str = strmerge(" > ", scommand_get_redir_out(self));
        char *new_str = strmerge(str_scmd, out_str);
        free(str_scmd);
        free(out_str); out_str = NULL;
        str_scmd = new_str;
    }

    // Tiene redireccionamiento in?
    if (self->in != NULL) {
        char *in_str = strmerge(" < ", scommand_get_redir_in(self));
        char *new_str = strmerge(str_scmd, in_str);
        free(str_scmd);
        free(in_str); in_str = NULL;
        str_scmd = new_str;
    }
    
    assert(scommand_is_empty(self) || scommand_get_redir_in(self) == NULL || scommand_get_redir_out(self) == NULL || strlen(str_scmd) > 0);

    return str_scmd;
}



//----- Implementaion pipeline (10) -----//
struct pipeline_s {
    GQueue *lcommands;  // Lista de scommand
    bool status;           // Se ejecuta de manera foreground o background?
};


pipeline pipeline_new(void) {
    pipeline new = malloc(sizeof(struct pipeline_s));
    new->lcommands = g_queue_new();
    new->status = true; /* por defecto todos los comandos seran ejecutados de manera foreground */

    assert(new != NULL && pipeline_is_empty(new) && pipeline_get_wait(new));
    return new;
}

// Funcion auxiliar para ser llamada en g_queue_free_full()
static void free_function_scommand(gpointer data) {
    if (data != NULL) {
        scommand kill_scommand = (scommand)data;
        // Se libera la memoria del scommand
        scommand_destroy(kill_scommand); kill_scommand = NULL; 
    }   
}
pipeline pipeline_destroy(pipeline self){
    assert (self != NULL);
    g_queue_free_full(self->lcommands,free_function_scommand);
    self->lcommands = NULL;
    free(self); self = NULL;
    assert(self == NULL);
    return self;
}


void pipeline_push_back(pipeline self, scommand sc){
    assert(self!=NULL && sc!=NULL);
    g_queue_push_tail(self->lcommands,sc);
    assert(!pipeline_is_empty(self));
}


void pipeline_pop_front(pipeline self){
    assert(self!=NULL && !pipeline_is_empty(self));    
    scommand popped_scommand = g_queue_pop_head(self->lcommands);
    scommand_destroy(popped_scommand);
}


void pipeline_set_wait(pipeline self, const bool w) {
    assert(self != NULL);
    self->status = w;
}


bool pipeline_is_empty(const pipeline self) {
    assert(self!=NULL);
    return g_queue_is_empty(self->lcommands);
}


unsigned int pipeline_length(const pipeline self) {
    assert(self!=NULL);
    unsigned int size = g_queue_get_length(self->lcommands);
    assert((g_queue_get_length(self->lcommands)==0) == pipeline_is_empty(self));
    return size;
}

scommand pipeline_front(const pipeline self) {
    assert(self!=NULL && !pipeline_is_empty(self));
    scommand front_scommand = g_queue_peek_head(self->lcommands);
    assert(front_scommand!= NULL);
    return front_scommand;
}


bool pipeline_get_wait(const pipeline self) {
    assert(self!=NULL);
    return self->status;
}


char * pipeline_to_string(const pipeline self){
    assert(self!=NULL);
    
    // Comenzamos con la cadena vacia (caso: pipeline vacio)
    char *str_pipe = (char *)malloc(1);
    str_pipe[0] = '\0';

    // Tiene comandos?
    if (!g_queue_is_empty(self->lcommands))
    {   
        guint length = g_queue_get_length(self->lcommands);
        for (guint i = 0; i < length; i++)
        {
            // Se transforman a string los scommand contenidos en el pipe
            char *element = scommand_to_string(g_queue_peek_nth(self->lcommands, i));
            char *new_str = strmerge(str_pipe, element);

            // Manejo de memoria
            free(element); element = NULL;
            free(str_pipe); str_pipe = NULL;
            str_pipe = new_str;
            
            // Si no es el ultimo comando se le agrega el operador pipe (|)
            if (i < length-1) {
                new_str = strmerge(str_pipe, " | ");
                free(str_pipe);
                str_pipe = new_str;
            }
        }

        // Lleva el operador de background?
        if (!(self->status)) {   
            char *new_str = strmerge(str_pipe, " &");
            free(str_pipe);
            str_pipe = new_str;
        }

        // Agregamos el caracter nulo al final
        str_pipe[strlen(str_pipe)] = '\0'; 
    }

    assert(pipeline_is_empty(self) || pipeline_get_wait(self) || strlen(str_pipe)>0);
    return str_pipe;
}
