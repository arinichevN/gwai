
void freeThread ( SerialThread *item ) {
	close(item->fd);
    freeMutex ( &item->mutex );
    FREE_LIST(&item->channelptr_list);
    free ( item );
}

void freeThreadList ( SerialThreadLList *list ) {
    SerialThread *item = list->top, *temp;
    while ( item != NULL ) {
        temp = item;
        item = item->next;
        freeThread ( temp );
    }
    list->top = NULL;
    list->last = NULL;
    list->length = 0;
}

void threadReset(SerialThread *item){
	close(item->fd); 
	FOREACH_LLIST (channelptr, &item->channelptr_llist, ChannelPtr ) {
		Channel *channel = channelptr->item;
		lockMutex(&channel->mutex);
		channel->thread = NULL;
		unlockMutex(&channel->mutex);
	}
	LLIST_RESET(&item->channelptr_llist);
	item->state = TERMINATED;
	//printdo("T %d TERMINATED\n", item->id);
}

int addChannelPtr ( ChannelPtr *item, ChannelPtrLList *list, ChannelPtrList *source  ) {
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

ChannelPtr * deleteChannelPtr ( ChannelPtr *item, ChannelPtrLList *list ) {
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
            lockMutex(&curr->item->mutex);
            curr->item->thread = NULL;
            unlockMutex(&curr->item->mutex);
          //  printdo ( "channelptr with id=%d has been DELETED from control list\n", item->item->id );
            return curr;
        }
        prev = curr;
    }
    return NULL;
}

int assignChannelToThread(ChannelPtr *item, SerialThread *thread){
	item->item->thread = thread;
	item->item->state = INIT;
	if(addChannelPtr(item, &thread->channelptr_llist, &thread->channelptr_list)){
		return 1;
	}
	return 0;
}

int channelExists ( int channel_id, int fd ) {
	printdo("Exists? id=%d\n", channel_id);
	int r = puart_sendCmd (fd, channel_id, "gfts");
	if(r == PUART_CONNECTION_FAILED){
		puart_sendEnd(fd);
		return PUART_CONNECTION_FAILED;
	}
	size_t resp_len = 128;
	char response[resp_len];
    memset(response, 0, resp_len);
	r = puart_readResponse(fd, response, resp_len);
	puart_sendEnd(fd);
    if(r < 0){
		printde("\tcommunication error where channel_id=%d\n", channel_id);
		return r;
	}
	int id=-1;
	int tm;
	double input;
	int state;
	r = sscanf(response, "%d" PUART_DELIMITER_BLOCK_STR "%lf" PUART_DELIMITER_BLOCK_STR "%d" PUART_DELIMITER_BLOCK_STR "%d" PUART_DELIMITER_END_STR, &id, &input, &tm, &state );
	int nr = 4;
	if(r != nr){
		printde("failed to parse response (found:%d, need:%d)\n", r, nr);
		return 0;
	}
	if(id != channel_id){
		printde("expected %d id but returned %d\n", channel_id, id);
		return 0;
	}
	return 1;
}

void searchNAddUnconnectedChannel(ChannelList *list, ChannelPtrLList *llist, SerialThread *thread) {
	for(int c=0;c < list->length;c++) {
		if(thread->chpl_ind >= list->length){
			//puts("search not needed");
			return;
		}
		Channel *channel = &list->item[thread->chpl_ind];
		lockMutex(&channel->mutex);
		int reset = 0;
		int done = 0;
		if(channel->thread == NULL) {
			printf("TRYIN TO CONNECT CHANNEL\n");
			int r = channelExists(channel->id, thread->fd);
			if(r==PUART_CONNECTION_FAILED){
				if(thread->retry < thread->max_retry){
					thread->retry++;
				}else{
					reset = 1;
				}
			}else{
				thread->retry = 0;
			}
			if(r == 1){
				assignChannelToThread(&thread->channelptr_list.item[thread->chpl_ind], thread);
			}
			done = 1;
			
		}
		unlockMutex(&channel->mutex);
		if(reset){
			threadReset(thread);
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

void threadControl(SerialThread *item){
	switch(item->state){
		case RUN:{//printdo("T %d RUN\n", item->id);
			searchNAddUnconnectedChannel(&channel_list, &item->channelptr_llist, item);
            FOREACH_LLIST (channelptr, &item->channelptr_llist, ChannelPtr ) {
				Channel *channel = channelptr->item;
                int r = channelControl(channel, item->fd);
                if(r==PUART_CONNECTION_FAILED){
					if(item->retry < item->max_retry){
						item->retry++;
					}else{
						threadReset(item);
						return;
					}
				}else{
					item->retry = 0;
				}
				if(r == PUART_NO_RESPONSE){
					deleteChannelPtr ( channelptr, &item->channelptr_llist);
				}
            }
            puts("");
			break;}
		case FIND_CHANNELS://printdo("T %d FIND_CHANNELS\n", item->id);
			FORLISTN(channel_list, i){
				Channel *channel = &channel_list.item[i];
				lockMutex(&channel->mutex);
				if(channel->thread == NULL){
					int r = channelExists( channel->id, item->fd);
					//int r = 1;
					if(r==PUART_CONNECTION_FAILED){
						if(item->retry < item->max_retry){
							item->retry++;
						}else{
							unlockMutex(&channel->mutex);
							threadReset(item);
							return;
						}
					}else{
						item->retry = 0;
					}
					if(r == 1){
						assignChannelToThread(&item->channelptr_list.item[i], item);
					}
				}
				unlockMutex(&channel->mutex);
			}
			if(item->channelptr_llist.length > 0){
				item->state = RUN;
			}
			break;
		case INIT:printf("T %d INIT\n", item->id);
			item->retry = 0;
			item->chpl_ind = 0;
		    item->state = FIND_CHANNELS;
			break;
		case TERMINATED:
			break;
		default:
			printde("bad state where thread.id = %d\n", item->id);
			break;
	}
}

void cleanup_handler_t ( void *arg ) {
    SerialThread *item = arg;
    printf ( "cleaning up thread %s\n", item->serial_path );
}

void *tThreadFunction ( void *arg ) {
    SerialThread *item = arg;
    printdo ( "thread with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
    pthread_cleanup_push ( cleanup_handler_t, item );
#endif
    while ( 1 ) {
        struct timespec t1 = getCurrentTime();
        int old_state;
        if ( threadCancelDisable ( &old_state ) ) {
            threadControl(item);
            threadSetCancelState ( old_state );
        }
        delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
    pthread_cleanup_pop ( 1 );
#endif
}
