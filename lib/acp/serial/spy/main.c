#include "main.h"

Acpsy *acpsy_newBegin(){
	size_t sz = sizeof (Acpsy);
	Acpsy *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new Acpsy\n");
		return self;
	}
	memset(self, 0, sz);
	if(!acpsypList_begin(&self->ports)){
		free(self);
		return NULL;
	}
	if(!acpsyidList_begin(&self->ids)){
		acpsypList_free(&self->ports);
		free(self);
		return NULL;
	}
	return self;
}

int acpsy_createNewPort(Acpsy *self, const char *serial_file_name, int serial_rate, const char *serial_dps) {
	if(!acpsypList_add(&self->ports, serial_file_name, serial_rate, serial_dps, &self->ids, ACPSY_MAX_PORT_COUNT)) {
		return 0;
	}
	return 1;
}

//int acpsy_addID(Acpsy *self, int id) {
	//AcpsyID *new_id = acpsyid_newBegin(id);
	//if(new_id == NULL) {
		//return 0;
	//}
	//if(!acpsyidLList_add(&self->ids, new_id, ACPSY_MAX_ID_COUNT)) {
		//acpsyid_free(new_id);
		//return 0;
	//}
	//return 1;
//}

int acpsy_addID(Acpsy *self, int id) {
	if(!acpsyidList_add(&self->ids, id, ACPSY_MAX_ID_COUNT)) {
		return 0;
	}
	return 1;
}

int acpsy_addCommandToID(Acpsy *self, int id, int command, struct timespec timeout){
	for(size_t i = 0; i < self->ids.length; i++){
		AcpsyID *oid = &self->ids.items[i];
		if(oid->value == id){
			return acpsyid_addCommand(oid, command, timeout, ACPSY_MAX_ID_COMMAND_COUNT);
		}
	}
	printde("id %d not found", id);
	return 0;
}

int acpsy_getResponse(Acpsy *self, int id, int command, char *out, size_t out_len){
	return 1;
}

void acpsy_free(Acpsy *self) {
	if (self == NULL) return;
	acpsypList_free(&self->ports);
	acpsyidList_free(&self->ids);
	free(self);
}

