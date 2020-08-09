#include "SerialThread.h"

void st_free ( SerialThread *item ) {
//puts("stoping serial threads...");
//	STOP_THREAD(item->thread)
//	puts("serial threads done...");
	close(item->fd);
    freeMutex ( &item->mutex );
    FREE_LIST(&item->channelptr_list);
    free ( item );
}

void stList_free ( SerialThreadLList *list ) {
    SerialThread *item = list->top, *temp;
    while ( item != NULL ) {
        temp = item;
        item = item->next;
        st_free ( temp );
    }
    list->top = NULL;
    list->last = NULL;
    list->length = 0;
}

void st_reset(SerialThread *item){
	close(item->fd); 
	FOREACH_LLIST (channelptr, &item->channelptr_llist, ChannelPtr ) {
		Channel *channel = channelptr->item;
		channel_resetData(channel);
	}
	LLIST_RESET(&item->channelptr_llist);
	item->state = TERMINATED;
	printdo("SERIAL THREAD %d TERMINATED\n", item->id);
}

SerialThread *stList_getIdleThread(SerialThreadLList *list){
	FORLLi(SerialThread){
		if(item->state == TERMINATED){
			return item;
		}
	}
	return NULL;
}

void st_setParam(SerialThread *item, char *serial_path, int fd, int serial_rate, struct timespec cycle_duration){
	strncpy(item->serial_path, serial_path, LINE_SIZE);
	item->fd = fd;
	item->serial_rate = serial_rate;
	item->cycle_duration = cycle_duration;
}

void st_start(SerialThread *item){
	item->state = INIT;				
}

int st_addToList (SerialThreadLList *list, Mutex *list_mutex, SerialThread *item ) {
    if ( list->length >= INT_MAX ) {
        printde ( "can not load thread with id=%d - list length exceeded\n", item->id );
        return 0;
    }
    if ( list->top == NULL ) {
        lockMutex ( list_mutex );
        list->top = item;
        unlockMutex ( list_mutex );
    } else {
        lockMutex ( &list->last->mutex );
        list->last->next = item;
        unlockMutex ( &list->last->mutex );
    }
    list->last = item;
    list->length++;
    printdo ( "thread with id=%d loaded\n", item->id );
    return 1;
}

SerialThread * st_deleteThread (SerialThreadLList *list, Mutex *list_mutex, SerialThread *item) {
    SerialThread *prev = NULL;
    FOREACH_LLIST ( curr,list,SerialThread ) {
        if ( curr == item ) {
            if ( prev != NULL ) {
                lockMutex ( &prev->mutex );
                prev->next = curr->next;
                unlockMutex ( &prev->mutex );
            } else {//curr=top
                lockMutex ( list_mutex );
                list->top = curr->next;
                unlockMutex ( list_mutex );
            }
            if ( curr == list->last ) {
                list->last = prev;
            }
            list->length--;
            return curr;
        }
        prev = curr;
    }
    return NULL;
}

int st_makeChannelPtrList(SerialThread *item, ChannelList *rlist){
	ChannelPtrList *list = &item->channelptr_list;
	ALLOC_LIST ( list, rlist->length)
    if ( list->max_length != rlist->length ) {
        putsde ( "failed to allocate memory for thread channel poiner list\n" );
        return 0;
    }
    FORMLi{
		list->item[i].item = &rlist->item[i];
		list->item[i].next = NULL;
		list->length++;
	}
	return 1;
}

//void st_useConnectionResult (SerialThread *item, int r){
	//if(r==ACP_ERROR_CONNECTION){
		//thread->retry++;
	//}else{
		//thread->retry = 0;
	//}
//}

int st_addChannelPtr ( ChannelPtr *item, ChannelPtrLList *list, ChannelPtrList *source  ) {
    if ( list->length >= source->length ) {
       // printde ( "can not add channelptr with id=%d - list length exceeded\n", item->item->id );
        return 0;
    }
    item->next = NULL;
    if ( list->top == NULL ) {
        list->top = item;
    } else {
        list->last->next = item;
    }
    list->last = item;
    list->length++;
    //printdo ( "channelptr with id=%d has been ADDED to control list\n", item->item->id );
    return 1;
}

ChannelPtr * st_deleteChannelPtr (SerialThread *thread, ChannelPtr *item) {
	ChannelPtrLList *list = &thread->channelptr_llist;
    ChannelPtr *prev = NULL;
    FOREACH_LLIST ( curr, list, ChannelPtr ) {
        if ( curr == item ) {
            if ( prev != NULL ) {
                prev->next = curr->next;
            } else {//curr=top
                list->top = curr->next;
            }
            if ( curr == list->last ) {
                list->last = prev;
            }
            list->length--;
          //  printdo ( "channelptr with id=%d has been DELETED from control list\n", item->item->id );
            return curr;
        }
        prev = curr;
    }
    return NULL;
}

int st_assignChannelToThread(SerialThread *thread, int ind ){
	ChannelPtr *item = &thread->channelptr_list.item[ind];
	item->item->thread = thread;
	item->item->state = INIT;
	if(st_addChannelPtr(item, &thread->channelptr_llist, &thread->channelptr_list)){
		return 1;
	}
	return 0;
}


int st_channelExists (int channel_id, int fd, Mutex *mutex) {
	lockMutex(mutex);
	int r = acpserial_sendChCmd (fd, channel_id, CMD_GET_ID_EXISTS);
	if(r == ACP_ERROR_CONNECTION){
		unlockMutex(mutex);
		return r;
	}
	size_t resp_len = 128;
	char response[resp_len];
    memset(response, 0, resp_len);
   ST_SLEEP_BEFORE_READ_SLAVE
    r = acpserial_readResponse(fd, response, resp_len);
    if(r != ACP_SUCCESS){
		printde("\tcommunication error where channel_id=%d\n", channel_id);
		unlockMutex(mutex);
		return r;
	}
	int id, exs = 0;
    r = acpserial_extractI2(response, resp_len, &id, &exs);
    if(r != ACP_SUCCESS){
		printde("\tparsing error where channel_id=%d\n", channel_id);
		unlockMutex(mutex);
		return r;
	}
	if(exs != ACP_CHANNEL_EXISTS){
		printde("channel %d returned not exists\n", channel_id);
		unlockMutex(mutex);
		return ACP_ERROR;
	}
    if(id != channel_id){
		printde("expected %d id but returned %d\n", channel_id, id);
		unlockMutex(mutex);
		return ACP_ERROR_BAD_CHANNEL_ID;
	}
	unlockMutex(mutex);
    return ACP_SUCCESS;
}


void st_searchNAddUnconnectedChannel(SerialThread *thread) {
	ChannelPtrList *list = &thread->channelptr_list;
	for(int c=0;c < list->length;c++) {
		if(thread->chpl_ind >= list->length){
			//puts("search not needed");
			return;
		}
		Channel *channel = list->item[thread->chpl_ind].item;
		lockMutex(&channel->mutex);
		int reset = 0;
		int done = 0;
		if(channel->thread == NULL) {
			printf("TRYIN TO CONNECT CHANNEL\n");
			int r = st_channelExists(channel->id, thread->fd, &thread->mutex);
			if(r==ACP_ERROR_CONNECTION){
				if(thread->retry < thread->max_retry){
					thread->retry++;
				}else{
					reset = 1;
				}
			}else{
				thread->retry = 0;
			}
			if(r == ACP_SUCCESS){
				st_assignChannelToThread(thread, thread->chpl_ind);
			}
			done = 1;
			
		}
		unlockMutex(&channel->mutex);
		if(reset){
			st_reset(thread);
		}
		if(thread->chpl_ind + 1 >= list->length) {
			thread->chpl_ind = 0;
		}else{
			thread->chpl_ind++;
		}
		if(done){
			printf("search channel_id %d checked\n", channel->id);
			break;
		}
	}
}

void st_control(SerialThread *item){
	switch(item->state){
		case RUN:{//printdo("SERIAL THREAD %d RUN\n", item->id);
			st_searchNAddUnconnectedChannel(item);
            FOREACH_LLIST (channelptr, &item->channelptr_llist, ChannelPtr ) {
				Channel *channel = channelptr->item;
                int r = channel_control(channel, item->fd);
                if(r==ACP_ERROR_CONNECTION){
					st_reset(item);
					return;
				}
				if(r == ACP_ERROR_NO_RESPONSE){
					channel_resetData(channel);
					st_deleteChannelPtr (item, channelptr);
					printdo("deleting channel id=%d from serial thread id=%d\n", channel->id, item->id);
				}
            }
            putsdo("");
			break;}
		case FIND_CHANNELS:printdo("SERIAL THREAD %d FIND_CHANNELS\n", item->id);
			//{int r = serial_canWrite(item->fd, 3000);
				//if(!r){sthreadReset(item);
					//return;}
				//r= serial_canRead(item->fd, 3000);
				//if(!r){sthreadReset(item);
					//return;}
			//}
			FORLISTN(item->channelptr_list, i){
				Channel *channel = item->channelptr_list.item[i].item;
				lockMutex(&channel->mutex);
				if(channel->thread == NULL){//unconnected channel
					int r = st_channelExists( channel->id, item->fd, &item->mutex);
					//int r = 1;
					if(r==ACP_ERROR_NO_RESPONSE){
						printdo("channel id=%d not found on serial port id=%d\n", channel->id, item->id);
						unlockMutex(&channel->mutex);
						continue;
					}
					if(r==ACP_ERROR_CONNECTION){
						if(item->retry < item->max_retry){
							item->retry++;
						}else{
							unlockMutex(&channel->mutex);
							st_reset(item);
							return;
							//continue;
						}
					}else{
						item->retry = 0;
					}
					if(r == ACP_SUCCESS){
						st_assignChannelToThread(item, i);
					}
				}
				unlockMutex(&channel->mutex);
			}
			if(item->channelptr_llist.length > 0){
				item->state = RUN;
			}
			break;
		case INIT:printf("SERIAL THREAD %d INIT\n", item->id);
			item->chpl_ind = 0;
			LLIST_RESET(&item->channelptr_llist);
		    item->state = FIND_CHANNELS;
			break;
		case TERMINATED:
			break;
		default:
			printde("bad state where thread.id = %d\n", item->id);
			break;
	}
}

#ifdef MODE_DEBUG
void st_cleanup_handler ( void *arg ) {
    SerialThread *item = arg;
    printf ( "cleaning up thread %s\n", item->serial_path );
}
#endif

void *st_main ( void *arg ) {
    SerialThread *item = arg;
    printdo ( "thread with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
    pthread_cleanup_push ( st_cleanup_handler, item );
#endif
    while ( 1 ) {
        struct timespec t1 = getCurrentTime();
        int old_state;
        if ( threadCancelDisable ( &old_state ) ) {
            st_control(item);
            threadSetCancelState ( old_state );
        }
        delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
    pthread_cleanup_pop ( 1 );
#endif
}

int st_addNewToList(SerialThreadLList *list, Mutex *list_mutex, ChannelList *rlist, int id, int max_retry, char *serial_path, int fd, int serial_rate, struct timespec cycle_duration){
	SerialThread *item = malloc ( sizeof * ( item ) );
    if ( item == NULL ) {
        putsde ( "failed to allocate memory for new thread\n" );
        return 0;
    }
    memset ( item, 0, sizeof *item );
    item->id = id;
    item->next = NULL;
    strncpy(item->serial_path, serial_path, LINE_SIZE);
    item->serial_rate = serial_rate;
    item->cycle_duration = cycle_duration;
    item->fd = fd;
    item->max_retry = max_retry;
    item->state = INIT;
    LIST_RESET(&item->channelptr_list)
    if(!st_makeChannelPtrList(item, rlist)){
		FREE_LIST(&item->channelptr_list);
		free ( item );
        return 0;
	}
	if ( !initMutex ( &item->mutex ) ) {
        free ( item );
        return 0;
    }
    if ( !st_addToList (list, list_mutex, item) ) {
        freeMutex ( &item->mutex );
        free ( item );
        return 0;
    }
	if ( !createMThread ( &item->thread, &st_main, item ) ) {
		freeMutex ( &item->mutex );
		free ( item );
		return 0;
	}
	return 1;
}
