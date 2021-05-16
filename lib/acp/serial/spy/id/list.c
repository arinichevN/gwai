#include "list.h"

//int acpsyidLList_begin(AcpsyIDLListm *self){
	//LLIST_RESET(self)
	//if(!mutex_init(&self->mutex)) {
		//return 0;
	//}
	//return 1;
//}

//int acpsyidLList_add(AcpsyIDLListm *self, AcpsyID *item, size_t items_max_count) {
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

//void acpsyidLList_free(AcpsyIDLListm *self){
	//AcpsyID *item = self->top, *temp;
	//while(item != NULL) {
		//temp = item;
		//item = item->next;
		//acpsyid_free(temp);
	//}
	//self->top = NULL;
	//self->last = NULL;
	//self->length = 0;
	//mutex_free(&self->mutex);
//}

int acpsyidList_begin(AcpsyIDList *self){
	LIST_RESET(self)
	return 1;
}

int acpsyidList_add(AcpsyIDList *self, int id, size_t items_max_count) {
	if(self->length >= items_max_count) {
		printde("can not add item to list: list length (%zu) limit (%zu) \n", self->length, items_max_count);
		return 0;
	}
	if(self->length == self->max_length){
		size_t new_length = self->length + ACPSY_ID_LIST_ALLOC_BLOCK_LENGTH;
		LIST_RESIZE(self, new_length)
		if(self->max_length != new_length){
			putsde("ids list realloc failed");
			return 0;
		}
	}
	if(acpsyid_begin(&self->items[self->length], id)){
		self->length++;
		return 1;
	}
	return 0;
}

void acpsyidList_free(AcpsyIDList *self){
	for (size_t i = 0; i < self->length; i++){
		acpsyid_free(&self->items[i]);
	}
	LIST_FREE(self)
}
