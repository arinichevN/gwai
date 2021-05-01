#include "main.h"

extern Acpsc *serial_client;

static int step_WAIT(NoidIntervalGetCommand *self, int remote_id);
static int step_OFF(NoidIntervalGetCommand *self, int remote_id);
static int step_FAILURE(NoidIntervalGetCommand *self, int remote_id);
static int step_INIT(NoidIntervalGetCommand *self, int remote_id);

void nigc_free(NoidIntervalGetCommand *self){
	mutex_lock(&self->command.mutex);
	mutex_unlock(&self->command.mutex);
	mutex_free(&self->command.mutex);
}

int nigcList_init(NoidIntervalGetCommandList *list, const char *dir, const char *file_name, const char *file_type){
	RESET_LIST(list)
	char path[LINE_SIZE];
	path[0] = '\0';
	strncat(path, dir, LINE_SIZE - 1);
	strncat(path, file_name, LINE_SIZE - strlen(path) - 1);
	strncat(path, file_type, LINE_SIZE - strlen(path) - 1);
	printf("iget path: %s\n", path);
	TSVresult *db = NULL;
	if(!TSVinit(&db, path)) {
		TSVclear(db);
		return 1;
	}
	int nt = TSVntuples(db);
	if(nt <= 0){
		TSVclear(db);
		putsdo("\tno noid poll commands\n");
		return 1;
	}
	ALLOC_LIST(list,nt);
	if(list->max_length!=nt){
		FREE_LIST(list);
		TSVclear(db);
		putsde("\tfailed to allocate memory for noid poll list\n");
		return 0;
	}
	for(int i = 0; i<nt; i++) {
		if(LL >= LML) break;
		int is = TSVgetis(db, i, "interval_s");
		int ins = TSVgetis(db, i, "interval_ns");
		char *command_str = TSVgetvalues(db, i, "command");
		if(TSVnullreturned(db)) {
			FREE_LIST(list);
			TSVclear(db);
			putsde("\tnull returned while reading noid_poll file 2\n");
			return 0;
		}
		if(is < 0 || ins < 0){
			FREE_LIST(list);
			TSVclear(db);
			putsde("\tnoid poll file: bad interval\n");
			return 0;
		}
		LIll.interval.tv_sec = is;
		LIll.interval.tv_nsec = ins;
		LIll.command.id = acp_commandStrToEnum(command_str);
		if(!mutex_init(&LIll.command.mutex)) {
			FREE_LIST(list);
			TSVclear(db);
			putsde("\tfailed to initialize noid command data mutex\n");
			return 0;
		}
		LIll.control = step_INIT;
		LL++;
		//printf("init poll LL++\n");
	}
	//printf("poll LL=%d\n",LL);
	TSVclear(db);
	return 1;
}

static void remoteGetRawData(NoidGetCommand *command, int remote_id){
	char request_str[ACP_BUF_MAX_LENGTH];
	memset(request_str, 0, sizeof request_str);
	if(!acpserial_buildPackII(request_str, ACP_BUF_MAX_LENGTH, ACP_SIGN_REQUEST_GET, command->id, remote_id)){
		mutex_lock(&command->mutex);
		command->result = ACP_ERROR_FORMAT;
		mutex_unlock(&command->mutex);
		return;
	}
	char response[ACP_BUF_MAX_LENGTH];
	memset(response, 0, sizeof response);
	int r = acpsc_getFromRemoteID(serial_client, remote_id, request_str, response, ACP_BUF_MAX_LENGTH);
	if(r == ACP_ERROR_CONNECTION){
		mutex_lock(&command->mutex);
		command->result = r;
		mutex_unlock(&command->mutex);
		return;
	}
	mutex_lock(&command->mutex);
	char *data = command->data;
	memset(data, 0, ACP_BUF_MAX_LENGTH * sizeof(*data));
	strncpy(data, response, ACP_BUF_MAX_LENGTH);
	acptcp_convertSerialPack(data);
	command->result = ACP_SUCCESS;
	printdo("iget data: %s\n", data);
	mutex_unlock(&command->mutex);
	return;
}

void nigc_reset(NoidIntervalGetCommand *self){
	self->command.result = 0;
}

static int step_WAIT(NoidIntervalGetCommand *self, int remote_id){
	if(tonr(&self->tmr)) {
		remoteGetRawData(&self->command, remote_id);     
	}
	return self->command.result;
}

static int step_OFF(NoidIntervalGetCommand *self, int remote_id){
	return self->command.result;
}

static int step_FAILURE(NoidIntervalGetCommand *self, int remote_id){
	return self->command.result;
}

static int step_INIT(NoidIntervalGetCommand *self, int remote_id){
	ton_setInterval(self->interval, &self->tmr);
	ton_reset(&self->tmr);
	self->control = step_WAIT;
	return self->command.result;
}

const char *nigc_getStateStr(NoidIntervalGetCommand *self){
	if(self->control == step_WAIT)         return "RUN";
	else if(self->control == step_OFF)     return "OFF";
	else if(self->control == step_FAILURE) return "FAILURE";
	else if(self->control == step_INIT)    return "INIT";
	return "?";
}
