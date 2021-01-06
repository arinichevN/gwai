#include "ACPSCIDLListm.h"

int acpscidLList_begin(ACPSCIDLListm *self){
	LLIST_RESET(self)
	if(!mutex_init(&self->mutex)) {
		return 0;
	}
	return 1;
}

int acpscidLList_add(ACPSCIDLListm *self, ACPSCID *item, size_t items_max_count) {
	if(self->length >= items_max_count) {
		printde("can not add item to list: list length (%zu) limit (%zu) \n", self->length, items_max_count);
		return 0;
	}
	if(self->top == NULL) {
		mutex_lock(&self->mutex);
		self->top = item;
		mutex_unlock(&self->mutex);
	} else {
		mutex_lock(&self->last->mutex);
		self->last->next = item;
		mutex_unlock(&self->last->mutex);
	}
	self->last = item;
	self->length++;
	return 1;
}

void acpscidLList_free(ACPSCIDLListm *self){
	ACPSCID *item = self->top, *temp;
	while(item != NULL) {
		temp = item;
		item = item->next;
		acpscid_free(temp);
	}
	self->top = NULL;
	self->last = NULL;
	self->length = 0;
	mutex_free(&self->mutex);
}
