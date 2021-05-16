#include "list.h"

//int acpsypList_begin(AcpsyPortList *self){
	//LLIST_RESET(self)
	//if(!mutex_init(&self->mutex)) {
		//return 0;
	//}
	//return 1;
//}

//int acpsypList_add(AcpsyPortList *self, AcpsyPort *item, size_t items_max_count) {
	//if(self->length >= items_max_count) {
		//printde("can not add item to list: list length (%zu) limit (%zu) \n", self->length, items_max_count);
		//return 0;
	//}
	//if(self->top == NULL) {
		//mutex_lock(&self->mutex);
		//self->top = item;
		//mutex_unlock(&self->mutex);
	//} else {
		//mutex_lock(&self->last->mutex);
		//self->last->next = item;
		//mutex_unlock(&self->last->mutex);
	//}
	//self->last = item;
	//self->length++;
	//return 1;
//}

//void acpsypList_free(AcpsyPortList *self){
	//AcpsyPort *item = self->top, *temp;
	//while(item != NULL) {
		//temp = item;
		//item = item->next;
		//acpsyp_free(temp);
	//}
	//self->top = NULL;
	//self->last = NULL;
	//self->length = 0;
	//mutex_free(&self->mutex);
//}

int acpsypList_begin(AcpsyPortList *self){
	LIST_RESET(self)
	return 1;
}

int acpsypList_add(AcpsyPortList *self, const char *serial_file_name, int serial_rate, const char *serial_dps, AcpsyIDList *ids, size_t items_max_count) {
	if(self->length >= items_max_count) {
		printde("\tcan not add item to list: list length (%zu) limit (%zu) \n", self->length, items_max_count);
		return 0;
	}
	if(self->length == self->max_length){
		size_t new_length = self->length + ACPSY_PORT_LIST_ALLOC_BLOCK_LENGTH;
		LIST_RESIZE(self, new_length)
		if(self->max_length != new_length){
			putsde("\tports list realloc failed\n");
			return 0;
		}
	}
	if(acpsyp_begin(&self->items[self->length], serial_file_name, serial_rate, serial_dps, ids)){
		self->length++;
		return 1;
	}
	return 0;
}

void acpsypList_free(AcpsyPortList *self){
	for (size_t i = 0; i < self->length; i++){
		acpsyp_free(&self->items[i]);
	}
	LIST_FREE(self)
}
