#include "main.h"

//AcpsyID *acpsyid_newBegin(int id){
	//size_t sz = sizeof (AcpsyID);
	//AcpsyID *self = malloc(sz);
	//if(self == NULL) {
		//putsde("failed to allocate memory for new AcpsyID\n");
		//return self;
	//}
	//memset(self, 0, sz);
	//if(!acpsygcLList_begin(&self->gcommands)){
		//free(self);
		//return NULL;
	//}
	//self->value = id;
	//self->next = NULL;
	//return self;
//}

int acpsyid_begin(AcpsyID *self, int id){
	memset(self, 0, sizeof *self);
	if(!acpsygcList_begin(&self->gcommands)){
		return 0;
	}
	self->value = id;
	printdo("id %d begin\n", id);
	return 1;
}

void acpsyid_free(AcpsyID *self) {
	acpsygcList_free(&self->gcommands);
}

//void acpsyid_free(AcpsyID *self) {
	//acpsygcLList_free(&self->gcommands);
	//free(self);
//}

int acpsyid_addCommand(AcpsyID *self, int command_id, struct timespec timeout, size_t items_max_count){
	return acpsygcList_add(&self->gcommands, command_id, timeout, items_max_count);
}
 
AcpsyGetCommand *acpsyid_getCommandById(AcpsyID *self, int command_value){
	for(size_t i = 0; i < self->gcommands.length; i++ ){
		AcpsyGetCommand *command = &self->gcommands.items[i];
		if(command->value == command_value){
			return command;
		}
	}
	return NULL;
}




