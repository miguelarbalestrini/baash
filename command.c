#include "command.h"
#include <glib.h>
#include <gmodule.h>
#include <assert.h>
#include "check.h"


typedef struct scommand_s
{
    GSList * cmd;
    bstring in;
    bstring out;
} scommand_s;

scommand scommand_new(void){
	scommand result = calloc (1,sizeof(struct scommand_s));
	result -> cmd = NULL;
	result -> in = NULL;
	result -> out = NULL;
	assert(result!=NULL && scommand_is_empty (result));
 	assert(scommand_get_redir_in (result) == NULL && scommand_get_redir_out (result) == NULL);
	return result;
}

scommand scommand_destroy(scommand self){
    assert(self!=NULL);
  	while (!scommand_is_empty(self)){
		scommand_pop_front(self);
  	}
	g_slist_free_full (self->cmd, free);
	bdestroy(self->in);
	bdestroy(self->out);
	free(self);
	return (self = NULL);
}

void scommand_push_back(scommand self, bstring argument){
    assert(self!=NULL && argument != NULL);
    self -> cmd = g_slist_append (self -> cmd, argument);
	assert(self -> cmd != NULL);
	assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
    assert(self!=NULL && !scommand_is_empty(self));
    bstring aux = NULL;
    aux = g_slist_nth_data (self -> cmd, 0);
    self-> cmd = g_slist_remove (self -> cmd, aux);
    bdestroy(aux);

}

void scommand_set_redir_in(scommand self, bstring filename){
	assert(self!=NULL);
    bdestroy(self->in);
    self->in = filename;
}

void scommand_set_redir_out(scommand self, bstring filename){
	assert(self!=NULL);
    bdestroy(self->out);
    self->out = filename;
}

bool scommand_is_empty(const scommand self){
	assert(self!=NULL);
	bool booleano = false;
	if (self -> cmd == NULL) {
		booleano = true;
	}
	return booleano;
}

unsigned int scommand_length(const scommand self){
    assert(self!=NULL);
    unsigned int length = 0;
    if(self->cmd == NULL){
        return length;
    }
	else{
        length = g_slist_length (self->cmd);
    }
    return length;
}

const_bstring scommand_front(const scommand self){
    assert(self!=NULL && !scommand_is_empty(self));
    const_bstring result = NULL;
    result = g_slist_nth_data( self -> cmd, 0);
    assert(result != NULL);
    return result;
}

const_bstring scommand_get_redir_in(const scommand self){
    assert(self!=NULL);
    return self -> in;
}

const_bstring scommand_get_redir_out(const scommand self){
    assert(self!=NULL);
    return self -> out;
}

bstring scommand_to_string(const scommand self){
	assert(self!=NULL);
	bstring result = bfromcstr("");
	for (unsigned int i = 0u;i<scommand_length(self);i++){
		bconcat(result,g_slist_nth_data(self->cmd,i));
		if(i!=scommand_length(self)-1){
			bconchar(result,(' '));
		}
	}
	if(scommand_get_redir_out(self)!=NULL){
		bconchar(result,(' '));
		bconchar(result,('>'));
		bconchar(result,(' '));
		bconcat(result,self-> out);
	}
	if(scommand_get_redir_in(self)!=NULL){
		bconchar(result,(' '));
		bconchar(result,('<'));
		bconchar(result,(' '));
		bconcat(result,self->in);
	}
	assert(scommand_is_empty(self) || scommand_get_redir_in(self)==NULL || scommand_get_redir_out(self)==NULL || blength(result)>0);
	return result;
}

typedef struct pipeline_s
{
	GSList * commands;
    bool pipeb;
} pipeline_s;

pipeline pipeline_new(void){
	pipeline result = calloc (1,sizeof(struct pipeline_s));
	result -> commands = NULL;
	result -> pipeb = true;
	assert(result != NULL && pipeline_is_empty(result) && pipeline_get_wait(result));
	return result;
}

pipeline pipeline_destroy(pipeline self){
	assert(self!=NULL);
	while (!pipeline_is_empty(self)){
		pipeline_pop_front(self);
	}
	g_slist_free_full (self->commands, free);
	free(self);
	return (self = NULL);
}

void pipeline_push_back(pipeline self, scommand sc){
	assert(self != NULL && sc != NULL);
	self -> commands = g_slist_append( self -> commands, sc);
	self -> pipeb = true;
	assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self){
	assert(self != NULL && !pipeline_is_empty(self));
	scommand aux;
	aux = g_slist_nth_data(self -> commands, 0);
	self -> commands = g_slist_remove (self -> commands,aux);
	scommand_destroy(aux);
	
}

void pipeline_set_wait(pipeline self, const bool w){
    assert(self!=NULL);
    if (self->pipeb == w){
        self->pipeb = true;

    }else{
        self->pipeb = false;
    }
}

bool pipeline_is_empty(const pipeline self){
    assert(self!=NULL);
    bool booleano = false;
    if (self->commands == NULL) {
        booleano = true;
    }
    return booleano;
}

unsigned int pipeline_length(const pipeline self){
    assert(self!=NULL);
    unsigned int length = 0;
    if(self->commands == NULL){
		assert(pipeline_is_empty(self));
        return length;
    }else{
    length = g_slist_length (self->commands);
    }
    return length;
}

scommand pipeline_front(const pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));
    scommand result = NULL;
    result = g_slist_nth_data(self -> commands, 0);
    assert(result != NULL);
    return result;
}

bool pipeline_get_wait(const pipeline self){
    assert(self!=NULL);
    pipeline_set_wait(self,&self->pipeb);
    return self->pipeb;
}

bstring pipeline_to_string(const pipeline self){
	assert(self != NULL);
	bstring result = bfromcstr("");
	unsigned int length = pipeline_length(self);
	for (unsigned int i = 1u;i<length;i++){
		bconcat(result,g_slist_nth_data(self->commands,i));
		if(i != length-1){
			bconchar(result,(' '));
		}
		if(pipeline_get_wait(self)==false){
			bconchar(result,(' '));
			bconchar(result,('|'));
			bconchar(result,(' '));
		}
	     if (pipeline_get_wait(self)==false){
            bconchar(result,(' '));
			bconchar(result,('&'));
			bconchar(result,(' '));
	    }

	}
	assert(pipeline_is_empty(self) || pipeline_get_wait(self) || blength(result)>0);
	return result;
}
