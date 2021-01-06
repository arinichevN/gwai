#include "main.h"

static void step_RUN ();
static void step_INIT ();
static void step_STOP ();
static void step_RESET ();
static void step_EXIT ();
static void step_OFF ();

void app_exit ();

void (*app_control)() = step_INIT;

int sock_port = -1;
int tcp_conn_num = 0;

ACPTS *tcp_server = NULL;
ACPSC *serial_client = NULL;
ChannelList channels = LIST_INITIALIZER;

void app_begin() {
#ifndef MODE_DEBUG
	daemon(0, 0);
#endif
	conSig(&app_exit);
}

int app_readSettings (const char *data_path, int *port, int *tcp_conn_num) {
	TSVresult *r = NULL;
	if (!TSVinit (&r, data_path)) {
		return 0;
	}
	int _port = TSVgetiById (r, "id", "value", "port");
	int _tcp_conn_num = TSVgetiById (r, "id", "value", "tcp_conn_num_max");
	if (TSVnullreturned (r)) {
		TSVclear (r);
		return 0;
	}
	*port = _port;
	*tcp_conn_num = _tcp_conn_num;
	TSVclear (r);
	return 1;
}

int app_beginSerialPorts(const char *config_path){
	TSVresult *r = NULL;
	if(!TSVinit(&r, config_path)) {
		TSVclear(r);
		return 1;
	}
	int n = TSVntuples(r);
	if(n <= 0){
		TSVclear(r);
		putsde("no data rows in file\n");
		return 0;
	}
	for(int i = 0; i < n; i++){
		char *name = TSVgetvalues(r, i, "name");
		int rate = TSVgetis(r, i, "rate");
		char *config = TSVgetvalues(r, i, "config");
		if(TSVnullreturned(r)) {
			break;
		}
		size_t fn = strlen(name) + strlen(OS_DEVICE_DIR) + 2;
		char path[fn] ;
		snprintf(path, fn, "%s/%s", OS_DEVICE_DIR, name) ;
		if(!acpsc_createNewPort(serial_client, path, rate, config)){
			printde("failed to create serial_port %s\n", path);
			TSVclear(r);
			return 0;
		}
	}
	TSVclear(r);
	return 1;
}

int app_beginChannels(const char *config_path, const char *get_dir, const char *iget_dir, const char *file_type){
	ChannelList *list = &channels;
	RESET_LIST(list)
	TSVresult *r = NULL;
	if(!TSVinit(&r, config_path)) {
		TSVclear(r);
		return 1;
	}
	int n = TSVntuples(r);
	if(n <= 0){
		TSVclear(r);
		putsde("no data rows in file\n");
		return 0;
	}
	ALLOC_LIST(list, n)
	if(list->max_length != n){
		TSVclear(r);
		putsde("failed to allocate memory for channel list\n");
		return 0;
	}
	for(int i = 0; i < LML; i++){
		LIST_RESET(&LIi.igcmd_list)
	}
	for(int i = 0; i < LML; i++){
		int id = TSVgetis(r, i, "id");
		char *iget_file = TSVgetvalues(r, i, "iget");
		if(TSVnullreturned(r)) {
			break;
		}
		if(!channel_setParam(&LIi, id, iget_file, iget_dir, file_type)){
			TSVclear(r);
			goto failed;
		}
		if(!acpsc_addRemoteID(serial_client, id)){
			TSVclear(r);
			goto failed;
		}
		LL++;
	}
	TSVclear(r);
	if(list->length != list->max_length){
		printde("check file %s: list.length=%zu but %zu expected\n", config_path, list->length, list->max_length);
		goto failed;
	}
	FORLi{
		if(!channel_begin(&LIi)) {
			goto failed;
		}
	}
	return 1;
	failed:
	channelList_free(list);
	return 0;
}

int app_init() {
	if (!app_readSettings (CONFIG_FILE, &sock_port, &tcp_conn_num)) {
		putsde ("failed to read settings\n");
		return 0;
	}
	tcp_server = acpts_newBegin(sock_port, tcp_conn_num, serveTCPRequest);
	if(tcp_server == NULL){
		putsde ("failed to initialize TCP server\n");
		return 0;
	}
	serial_client = acpsc_newBegin();
	if(serial_client == NULL){
		putsde ("failed to initialize serial server\n");
		return 0;
	}
	if(!app_beginSerialPorts(SERIAL_PORTS_CONFIG_FILE)){
		putsde ("failed to initialize serial ports\n");
		return 0;
	}
	if(!app_beginChannels(CHANNELS_CONFIG_FILE, CHANNELS_GET_DIR, CHANNELS_IGET_DIR, CONF_FILE_TYPE)){
		putsde ("failed to initialize channels\n");
		return 0;
	}
	return 1;
}

void app_free() {
	channelList_free(&channels);
	acpts_free(tcp_server);
	tcp_server = NULL;
	acpsc_free(serial_client);
	serial_client = NULL;
}

void app_exit () {
	app_free();
	putsdo ("\nexiting now...\n");
	exit (EXIT_SUCCESS);
}

void app_reset(){
	app_control = step_RESET;
}

const char *getAppState(){
	if(app_control == step_RUN)				return "RUN";
	else if(app_control == step_INIT)		return "INIT";
	else if(app_control == step_STOP)		return "STOP";
	else if(app_control == step_RESET)		return "RESET";
	else if(app_control == step_EXIT)		return "EXIT";
	else if(app_control == step_OFF)			return "OFF";
	return "?";
}

static void step_RUN (){
	NANOSLEEP(0, 10000000);
}

static void step_INIT(){
	if (!app_init()) {
		exit (EXIT_FAILURE);
	}
	app_control = step_RUN;
}

static void step_STOP() {
	app_control = step_OFF;
}

static void step_RESET(){
	app_free();
	NANOSLEEP(0, 100000000);
	app_control = step_INIT;
}

static void step_EXIT(){
	app_exit();
}

static void step_OFF(){
	NANOSLEEP(0, 10000000);
}

