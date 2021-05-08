#include "main.h"

AcpscID *acpscid_newBegin(int id){
	size_t sz = sizeof (AcpscID);
	AcpscID *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new AcpscID\n");
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

void acpscid_free(AcpscID *self) {
	mutex_free(&self->mutex);
	free(self);
}

void acpscid_lock(AcpscID *self){
	mutex_lock(&self->mutex);
}

void acpscid_unlock(AcpscID *self){
	mutex_unlock(&self->mutex);
}

//lock object before using this function
int acpscid_isOnline(AcpscID *self){
	if(self->owner != NULL && self->state == ACPSCID_FOUND) return 1;
	return 0;
}

int acpscid_setOwner(AcpscID *self, void *owner){
	if(self->owner == NULL){
		self->owner = owner;
		self->state = ACPSCID_FOUND;
		return 1;
	} else if (self->owner == owner){
		self->state = ACPSCID_FOUND;
		return 1;
	}
	printde("duplicate id=%d found on other port", self->value);
	return 0;
	
}

int acpscid_needSearchOnPort(AcpscID *self, void *owner) {
	if((self->owner == NULL && self->state == ACPSCID_NOT_FOUND) || (self->owner == owner && self->state == ACPSCID_PORT_DISCONNECTED)){
		return 1;
	}
	return 0;
}

const char *acpscid_getStateStr(AcpscID *self){
	switch(self->state){
		case ACPSCID_NOT_FOUND:				return "NOT_FOUND";
		case ACPSCID_FOUND:					return "FOUND";
		case ACPSCID_PORT_DISCONNECTED:		return "PORT?";
	}
	return "?";
}
