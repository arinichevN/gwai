
void app_RUN ();
void app_INIT ();
void app_INIT_DATA ();
void app_STOP ();
void app_RESET ();
void app_EXIT ();
void app_OFF ();

void (*control)() = app_INIT;

int sock_port = -1;
int conn_num = 0;
int max_retry;

int app_readSettings ( const char *data_path, int *port, struct timespec *cd, int *conn_num, int *max_retry, int *serial_rate, char *serial_config, char *serial_pattern) {
    TSVresult *r = NULL;
    if ( !TSVinit ( &r, data_path ) ) {
        return 0;
    }
    int _port = TSVgetiById ( r, "id", "value", "port" );
    int _cd_s = TSVgetiById ( r, "id", "value", "cd_s" );
    int _cd_ns = TSVgetiById ( r, "id", "value", "cd_ns" );
    int _conn_num = TSVgetiById ( r, "id", "value", "conn_num_max" );
    int _max_retry = TSVgetiById ( r, "id", "value", "max_retry" );
    int _serial_rate = TSVgetiById ( r, "id", "value", "serial_rate" );
    char *_serial_config = TSVgetvalueById ( r, "id", "value", "serial_config" );
    char *_serial_pattern = TSVgetvalueById ( r, "id", "value", "serial_pattern" );
    if ( TSVnullreturned ( r ) ) {
		TSVclear ( r );
        return 0;
    }
    *port = _port;
    cd->tv_sec = _cd_s;
    cd->tv_nsec = _cd_ns;
    *conn_num = _conn_num;
    *max_retry = _max_retry;
    *serial_rate = _serial_rate;
    strncpy(serial_config, _serial_config, LINE_SIZE);
    strncpy(serial_pattern, _serial_pattern, LINE_SIZE);
    TSVclear ( r );
    return 1;
}


int app_init() {
    if ( !app_readSettings (CONFIG_FILE, &sock_port, &serial_thread_starter.thread_cd, &conn_num, &max_retry,  &serial_thread_starter.serial_rate,  serial_thread_starter.serial_config, serial_thread_starter.serial_pattern) ) {
        putsde ( "failed to read settings\n" );
        return 0;
    }
    if ( !initMutex (&serial_thread_list_mutex) ) {
        putsde ( "failed to initialize serial_thread_list mutex\n" );
        return 0;
    }
    if ( !serverm_init(&server, sock_port, conn_num, serveRequest)){
		putsde ( "failed to initialize multythreaded server\n" );
        return 0;
	 }
    return 1;
}


int app_initModel() {
    if ( !channelList_init ( &channel_list, max_retry, CHANNELS_CONFIG_FILE, CHANNELS_GET_DIR, CHANNELS_IGET_DIR, CHANNELS_TGET_DIR, CHANNELS_SET_DIR, CONF_FILE_TYPE) ) {
        channelList_free ( &channel_list );
        goto failed;
    }
    if ( !sts_init ( &serial_thread_starter, max_retry ) ) {
		channelList_free ( &channel_list );
        goto failed;
    }
    return 1;
failed:
    return 0;
}

void app_freeModel() {
	STOP_ALL_LLIST_THREADS(&serial_thread_list, SerialThread)
	channelList_free ( &channel_list );
	stList_free ( &serial_thread_list );
	sts_free(&serial_thread_starter);
}

void app_free() {
    app_freeModel();
    serverm_free(&server);
    freeMutex ( &serial_thread_list_mutex );
}

void app_exit ( ) {
    app_free();
    putsdo ( "\nexiting now...\n" );
    exit ( EXIT_SUCCESS );
}

void app_reset(){
	control = app_RESET;
}

const char *getAppState(){
	if(control == app_RUN)				return "RUN";
	else if(control == app_INIT)		return "INIT";
	else if(control == app_INIT_DATA)	return "INIT_DATA";
	else if(control == app_STOP)		return "STOP";
	else if(control == app_RESET)		return "RESET";
	else if(control == app_EXIT)		return "EXIT";
	else if(control == app_OFF)			return "OFF";
	return "?";
}

void app_RUN (){
	nanosleep ( &(struct timespec) {0,10000000}, NULL );
}

void app_INIT (){
	if ( !app_init() ) {
		exit ( EXIT_FAILURE );
	}
	control = app_INIT_DATA;
}

void app_INIT_DATA (){
	app_initModel();
	control = app_RUN;
	delayUsIdle ( 1000000 );
}

void app_STOP (){
	app_freeModel();
	control = app_OFF;
}

void app_RESET (){
	app_free();
	delayUsIdle ( 1000000 );
	control = app_INIT;
}

void app_EXIT (){
	app_exit();
}

void app_OFF (){
	nanosleep ( &(struct timespec) {0,10000000}, NULL );
}

