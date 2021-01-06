#include "list.h"

void channelList_free (ChannelList *list){
	FORLi{
		channel_free(&LIi);
	}
	FREE_LIST(list);
}

