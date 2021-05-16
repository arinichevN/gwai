#include "list.h"

void noidList_terminate(NoidList *list){
	FORLi{
		noid_terminate(&LIi);
	}
}

void noidList_free(NoidList *list){
	FORLi{
		noid_free(&LIi);
	}
	LIST_FREE(list);
}

