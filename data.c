
int getChannelList_callback ( void *data, int argc, char **argv, char **azColName ) {
    ChannelList *list = data;
    Channel *item = &list->item[list->length];
    int c = 0;
    DB_FOREACH_COLUMN {
        if ( DB_COLUMN_IS ( "id" ) ) {
            item->id = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "run_interval_s" ) ) {
            item->run.interval.tv_sec = DB_CVI;
            item->run.interval.tv_nsec = 0;
            c++;
        } else {
            printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
        }
    }
    int n = 2;
    if ( c != n ) {
        printde ( "required %d columns but %d found\n", n, c );
        return EXIT_FAILURE;
    }
    list->length++;
    return EXIT_SUCCESS;
}


int getChannelListByIdFromDB ( ChannelList *list,  sqlite3 *db ) {
    char *q = "select * from channel";
    if ( !db_exec ( db, q, getChannelList_callback, list ) ) {
        putsde ( " failed\n" );
        return 0;
    }
    return 1;
}

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

int initChannelList ( ChannelList *list, int max_retry, sqlite3 *db ) {
	RESET_LIST ( list )
    int n = 0;
    char *qn = "select count(*) FROM channel";
    db_getInt ( &n, db, qn );
    if ( n <= 0 ) {
        return 1;
    }
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
        putsde ( "failed to allocate memory for channel list\n" );
        return 0;
    }
    if ( !getChannelListByIdFromDB ( list, db ) ) {
		FREE_LIST ( list );
        return 0;
    }
    if ( list->length != list->max_length ) {
        printde ( "list.length=%u but %u expected\n", list->length, list->max_length );
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

