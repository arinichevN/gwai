
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
        FORLISTN(LIi.data_list, j){
            freeMutex ( &LIi.data_list.item[j].mutex );
        }
        FREE_LIST(&LIi.data_list);
        FREE_LIST(&LIi.set_list);
		freeMutex ( &LIi.mutex );
	}
	FREE_LIST(list);
}

int setDataByType(const char *dtype, SlaveDataItem *item){
    if(  strncmp(dtype, SLAVE_TYPE_FTS_STR, SLAVE_TYPE_MAX_LENGTH * sizeof(*dtype)) == 0 ) {
        item->readFunction = channelSlaveGetFTSData;
        item->sendFunction = channelSendFTSData;
        item->data_type = SLAVE_TYPE_FTS;
    } else if(strncmp(dtype, SLAVE_TYPE_INT_STR, SLAVE_TYPE_MAX_LENGTH * sizeof(*dtype)) == 0 ) {
        item->readFunction = channelSlaveGetIntData;
        item->sendFunction = channelSendIntData;
        item->data_type = SLAVE_TYPE_INT;
    } else if(strncmp(dtype, SLAVE_TYPE_DOUBLE_STR, SLAVE_TYPE_MAX_LENGTH * sizeof(*dtype)) == 0 ) {
        item->readFunction = channelSlaveGetFloatData;
        item->sendFunction = channelSendFloatData;
        item->data_type = SLAVE_TYPE_DOUBLE;
    } else {
        item->readFunction = NULL;
        item->sendFunction = NULL;
        item->data_type = SLAVE_TYPE_UNKNOWN;
        printde("unknown slave data type: %s\n", dtype);
        return 0;
    }
    return 1;
}

int setSetCmdByType(const char *dtype, SlaveSetItem *item){
    if(strncmp(dtype, SLAVE_TYPE_INT_STR, SLAVE_TYPE_MAX_LENGTH * sizeof(*dtype)) == 0 ) {
        item->setFunction = channelSlaveSetIntData;
        item->data_type = SLAVE_TYPE_INT;
    } else if(strncmp(dtype, SLAVE_TYPE_DOUBLE_STR, SLAVE_TYPE_MAX_LENGTH * sizeof(*dtype)) == 0 ) {
        item->setFunction = channelSlaveSetFloatData;
        item->data_type = SLAVE_TYPE_DOUBLE;
    } else {
        item->setFunction = NULL;
        item->data_type = SLAVE_TYPE_UNKNOWN;
        printde("unknown slave data type: %s\n", dtype);
        return 0;
    }
    return 1;
}
int initChannelPoll(char *file_name, int channel_id, SlaveDataItemList *list ){
	RESET_LIST ( list )
	char path[LINE_SIZE];
	path[0] = '\0';
	strncat(path, CHANNELS_GET_DIR, LINE_SIZE - 1);
	strncat(path, file_name, LINE_SIZE - strlen(path) - 1);
	strncat(path, CONF_FILE_TYPE, LINE_SIZE - strlen(path) - 1);
	printf("get path: %s\n", path);
	TSVresult *db = NULL;
    if ( !TSVinit ( &db, path ) ) {
        TSVclear ( db );
        return 0;
    }
    int nt = TSVntuples ( db );
    if(nt <= 0 ){
		TSVclear ( db );
		printdo("no slave poll commands for channel_id=%d\n", channel_id);
        return 1;
    }
    ALLOC_LIST ( list,nt );
    if ( list->max_length!=nt ) {
		FREE_LIST(list);
		TSVclear ( db );
        putsde ( "failed to allocate memory for channel poll list\n" );
        return 0;
    }
    for(int i = 0; i<nt; i++) {
		if(LL >= LML) break;
		int is = TSVgetis ( db, i, "interval_s" );
		int ins = TSVgetis ( db, i, "interval_ns" );
		char *cmd = TSVgetvalues(db, i, "cmd");
		char *data_type = TSVgetvalues(db, i, "data_type");
		if ( TSVnullreturned ( db ) ) {
			FREE_LIST(list);
			TSVclear ( db );
			putsde("null returned while reading channel_poll file 2\n");
			return 0;
		}
		if(is < 0 || ins < 0){
			FREE_LIST(list);
			TSVclear ( db );
			printde("channel poll file: bad interval where channel_id = %d\n", channel_id);
			return 0;
		}
		LIll.interval.tv_sec = is;
		LIll.interval.tv_nsec = ins;
		strncpy(LIll.cmd, cmd, SLAVE_CMD_MAX_SIZE);
		if(!setDataByType(data_type, &LIll)){
			FREE_LIST(list);
			TSVclear ( db );
			printde("   at row %d in %s\n", i, path);
			return 0;
		}
		if ( !initMutex ( &LIll.mutex ) ) {
			FREE_LIST ( list );
			TSVclear ( db );
			putsde ( "failed to initialize slave data mutex\n" );
			return 0;
		}
		LIll.state = INIT;
		LL++;
		//printf("init poll LL++\n");
    }
    //printf("poll LL=%d\n",LL);
    TSVclear ( db );
    return 1;
}

int initChannelSetCmd(char *file_name, int channel_id, SlaveSetItemList *list ){
    RESET_LIST ( list )
	char path[LINE_SIZE];
	path[0] = '\0';
	strncat(path, CHANNELS_SET_DIR, LINE_SIZE - 1);
	strncat(path, file_name, LINE_SIZE - strlen(path) - 1);
	strncat(path, CONF_FILE_TYPE, LINE_SIZE - strlen(path) - 1);
	printf("set path: %s\n", path);
	TSVresult *db = NULL;
    if ( !TSVinit ( &db, path ) ) {
        TSVclear ( db );
        return 0;
    }
    int nt = TSVntuples ( db );
    if(nt <= 0 ){
		TSVclear ( db );
		printdo("no slave set commands for channel_id=%d\n", channel_id);
        return 1;
    }
    ALLOC_LIST ( list,nt )
    if ( list->max_length!=nt ) {
		FREE_LIST(list);
		TSVclear ( db );
        putsde ( "failed to allocate memory for channel poll list\n" );
        return 0;
    }
    for(int i = 0;i<nt;i++){
		if(LL >= LML) break;
		char *cmd = TSVgetvalues(db, i, "cmd");
		char *data_type = TSVgetvalues(db, i, "data_type");
		if ( TSVnullreturned ( db ) ) {
			FREE_LIST(list);
			TSVclear ( db );
			putsde("null returned while reading channel_poll file 2\n");
			return 0;
		}
		strncpy(LIll.cmd, cmd, SLAVE_CMD_MAX_SIZE);
		if(!setSetCmdByType(data_type, &LIll)){
			FREE_LIST(list);
			TSVclear ( db );
			printde("   at row %d in %s\n", i, path);
			return 0;
		}
		LL++;
    }
    TSVclear ( db );
    return 1;
}

int initChannelList ( ChannelList *list, int max_retry, const char *config_path ) {
	RESET_LIST ( list )
    TSVresult *r = NULL;
    if ( !TSVinit ( &r, config_path ) ) {
        TSVclear ( r );
        return 0;
    }
    int n = TSVntuples ( r );
    if ( n <= 0 ) {
        TSVclear ( r );
        putsde ( "no data rows in file\n" );
        return 0;
    }
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
		TSVclear ( r );
        putsde ( "failed to allocate memory for channel list\n" );
        return 0;
    }
    for ( int i = 0; i < LML; i++ ) {
        LIi.id = TSVgetis ( r, i, "id" );
        char *get_path = TSVgetvalues ( r, i, "get" );
        char *set_path = TSVgetvalues ( r, i, "set" );
        if ( TSVnullreturned ( r ) ) {
            break;
        }
        if(!initChannelPoll(get_path, LIi.id, &LIi.data_list)){
			TSVclear ( r );
            FREE_LIST ( list );
            return 0;
        }
        if(!initChannelSetCmd(set_path, LIi.id, &LIi.set_list)){
			TSVclear ( r );
            FREE_LIST ( list );
            return 0;
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
		LIi.max_retry = max_retry;
		LIi.state = INIT;
		if ( !initMutex ( &LIi.mutex ) ) {
	        FREE_LIST ( list );
	        putsde ( "failed to initialize channel mutex\n" );
	        return 0;
	    }
		if ( !checkChannel ( &LIi ) ) {
			freeMutex ( &LIi.mutex );
	        FREE_LIST ( list );
	        return 0;
	    }
	}
    return 1;
}

