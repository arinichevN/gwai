#include "ACPSS.h"

static int addPort(ACPSS *self, ACPSSPort *item) {
	ACPSSPortLListm *list = &self->ports;
	if(list->length >= ACPSS_MAX_PORT_COUNT) {
		printde("can not add port: list length (%zu) limit (%lu) \n", list->length, ACPSS_MAX_PORT_COUNT);
		return 0;
	}
	if(list->top == NULL) {
		mutex_lock(&list->mutex);
		list->top = item;
		mutex_unlock(&list->mutex);
	} else {
		mutex_lock(&list->last->mutex);
		list->last->next = item;
		mutex_unlock(&list->last->mutex);
	}
	list->last = item;
	list->length++;
	putsdo("port added to list\n");
	return 1;
}

ACPSS *acpss_new(){
	size_t sz = sizeof (ACPSS);
	ACPSS *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new ACPS\n");
		return self;
	}
	memset(self, 0, sz);
	LLIST_RESET(&self->ports)
	if(!mutex_init(&self->ports.mutex)) {
		free(self);
		return NULL;
	}
	return self;
}

int acpss_createNewPort(ACPSS *self, const char *serial_file_name, int serial_rate, const char *serial_config, ACPSSCommandNode *acnodes, size_t acnodes_count){
	ACPSSPort *new_port = acpssp_newBegin(serial_file_name, serial_rate, serial_config, acnodes, acnodes_count);
	if(new_port == NULL) {
		return 0;
	}
	if(!addPort(self, new_port)) {
		acpssp_free(new_port);
		return 0;
	}
	return 1;
}

void acpss_free(ACPSS *self){
	if (self == NULL) return;
	ACPSSPortLListm *list = &self->ports;
	ACPSSPort *item = list->top, *temp;
	while(item != NULL) {
		temp = item;
		item = item->next;
		acpssp_free(temp);
	}
	list->top = NULL;
	list->last = NULL;
	list->length = 0;
	mutex_free(&list->mutex);
	free(self);
}
