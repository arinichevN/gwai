#include "ACPSCID.h"

ACPSCID *acpscid_newBegin(int id){
	size_t sz = sizeof (ACPSCID);
	ACPSCID *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new ACPSCID\n");
		return self;
	}
	memset(self, 0, sz);
	if(!mutex_init(&self->mutex)) {
		free(self);
		return NULL;
	}
	self->value = id;
	self->state = ACPSCID_NOT_FOUND;
	self->owner = NULL;
	self->next = NULL;
	return self;
}

void acpscid_free(ACPSCID *self) {
	mutex_free(&self->mutex);
	free(self);
}

void acpscid_lock(ACPSCID *self){
	mutex_lock(&self->mutex);
}

void acpscid_unlock(ACPSCID *self){
	mutex_unlock(&self->mutex);
}

//lock object before using this function
int acpscid_isOwned(ACPSCID *self){
	if(self->owner != NULL) return 1;
	return 0;
}

void acpscid_setOwner(ACPSCID *self, void *owner){
	self->owner = owner;
}

const char *acpscid_getStateStr(ACPSCID *self){
	switch(self->state){
		case ACPSCID_NOT_FOUND:				return "NOT_FOUND";
		case ACPSCID_FOUND:					return "FOUND";
		case ACPSCID_PORT_DISCONNECTED:		return "PORT?";
	}
	return "?";
}
