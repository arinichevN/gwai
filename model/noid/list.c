#include "list.h"

void noidList_free (NoidList *list){
	FORLi{
		noid_free(&LIi);
	}
	FREE_LIST(list);
}

