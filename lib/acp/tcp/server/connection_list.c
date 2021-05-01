#include "connection_list.h"

void acptsconnList_free(AcptsConnectionLList *list){
	AcptsConnection *item = list->top;
	while(item != NULL){
		AcptsConnection *temp = item;
		item = item->next;
		acptsconn_free(temp);
	}
	list->top = NULL;
	list->last = NULL;
	list->length = 0;
}

int acptsconnList_push(AcptsConnectionLList *list, AcptsConnection *item){
	size_t prev_length = list->length;
	LLIST_ADD_ITEM(item, list)
	if((prev_length + 1) != list->length){
		putsde("failed to add new connection to list\n");
		return 0;
	}
	return 1;
}
