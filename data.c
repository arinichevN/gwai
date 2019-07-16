
int checkChannel ( Channel *item ) {
    int success = 1;
    if ( 0 ) {
        printde ( "invalid: channel where id=%d\n", item->id );
        success = 0;
    }
    return success;
}

void freeChannelList (ChannelList *list){
	FORLi{
		freeMutex ( &LIi.run.mutex );
		freeMutex ( &LIi.mutex );
	}
	FREE_LIST(list);
}

int initChannelList ( ChannelList *list, int max_retry, const char *config_path ) {
	TSVresult tsv = TSVRESULT_INITIALIZER;
    TSVresult* r = &tsv;
    if ( !TSVinit ( r, config_path ) ) {
        TSVclear ( r );
        return 0;
    }
    int n = TSVntuples ( r );
    if ( n <= 0 ) {
        TSVclear ( r );
        putsde ( "no data rows in file\n" );
        return 0;
    }
	RESET_LIST ( list )
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
		TSVclear ( r );
        putsde ( "failed to allocate memory for channel list\n" );
        return 0;
    }
    for ( int i = 0; i < LML; i++ ) {
        LIi.id = TSVgetis ( r, i, "id" );
        LIi.run.interval.tv_sec = TSVgetis ( r, i, "interval_s" );
        LIi.run.interval.tv_sec = TSVgetis ( r, i, "interval_ns" );
        if(LIi.run.interval.tv_sec < 0 || LIi.run.interval.tv_nsec < 0){
			printde("check file %s: bad interval where id = %d\n", config_path, LIi.id);
			break;
		}
        if ( TSVnullreturned ( r ) ) {
            break;
        }
        LL++;
    }
    TSVclear ( r );
    if ( list->length != list->max_length ) {
        printde ( "check file %s: list.length=%u but %u expected\n", config_path, list->length, list->max_length );
        FREE_LIST ( list );
        return 0;
    }
    FORLi{
		LIi.thread = NULL;
		LIi.run.input_state = 0;
		LIi.max_retry = max_retry;
		LIi.state = INIT;
		if ( !initMutex ( &LIi.mutex ) ) {
	        FREE_LIST ( list );
	        putsde ( "failed to initialize channel mutex\n" );
	        return 0;
	    }
	    if ( !initMutex ( &LIi.run.mutex ) ) {
			freeMutex ( &LIi.mutex );
	        FREE_LIST ( list );
	        putsde ( "failed to initialize channel mutex\n" );
	        return 0;
	    }
		if ( !checkChannel ( &LIi ) ) {
			freeMutex ( &LIi.run.mutex );
			freeMutex ( &LIi.mutex );
	        FREE_LIST ( list );
	        return 0;
	    }
	}
    return 1;
}

