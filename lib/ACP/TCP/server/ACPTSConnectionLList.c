#include "ACPTSConnectionLList.h"

void acptsconnList_free(ACPTSConnectionLList *list){
	ACPTSConnection *item = list->top;
	while(item != NULL){
		ACPTSConnection *temp = item;
		item = item->next;
		acptsconn_free(temp);
	}
	list->top = NULL;
	list->last = NULL;
	list->length = 0;
}

int acptsconnList_push(ACPTSConnectionLList *list, ACPTSConnection *item){
	size_t prev_length = list->length;
	LLIST_ADD_ITEM(item, list)
	if((prev_length + 1) != list->length){
		putsde("failed to add new connection to list\n");
		return 0;
	}
	return 1;
}
