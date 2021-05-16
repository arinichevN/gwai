#include "main.h"

static void step_RUN(AcpsyPort *self);
static void step_TRY_OPEN(AcpsyPort *self);

static void gotoOpen(AcpsyPort *self){
	close(self->fd);
	self->cycle_duration = ACPSYP_CYCLE_DURATION_TRY_OPEN;
	self->control = step_TRY_OPEN;
	printdo("port %s disconnected, trying to open it again...\n", self->param.filename);
}

static void step_TRY_OPEN(AcpsyPort *self){
	thread_cancelDisable();
	if (!serial_init(&self->fd, self->param.filename, self->param.rate, self->param.dps)) {
		printde("failed to open serial: %s %d %s\n", self->param.filename, self->param.rate, self->param.dps);
		thread_cancelEnable();
		return;
	}
	printdo("serial opened: %s %d %s\n", self->param.filename, self->param.rate, self->param.dps);
	self->cycle_duration = ACPSYP_CYCLE_DURATION_RUN;
	self->control = step_RUN;
	thread_cancelEnable();
}

static void step_RUN(AcpsyPort *self){
	char request[ACP_BUF_MAX_LENGTH];
	memset(request, 0, sizeof request);
	switch(acpserial_readRequest(self->fd, request, ACP_BUF_MAX_LENGTH)){
		case ACP_SUCCESS: break;
		case ACP_ERROR_DEVICE_DISCONNECTED:
			gotoOpen(self);
			return;
		default: return;
	}
	char sign = request[ACP_IND_SIGN];
	if(sign != ACP_SIGN_REQUEST_GET){
		putsdo("this is not get-command\n");
		return;
	}
	int noid;
	if(!acp_packGetCellI(request, NOID_ACP_REQUEST_IND_ID, &noid)){
		putsde("failed to get id\n");
		return;
	}
	AcpsyGetCommand *gcommand = NULL;
	for(size_t i = 0; i < self->ids->length; i++){
		AcpsyID *id = &self->ids->items[i];
		if(id->value == noid){
			int cmd;
			if(!acp_packGetCellI(request, ACP_REQUEST_IND_CMD, &cmd)){
				putsde("failed to get command\n");
				return;
			}
			gcommand = acpsyid_getCommandById(id, cmd);
			break;
		}
	}
	if(gcommand == NULL){
		putsdo("not for me\n");
		return;
	}
	//NANOSLEEP(0, 100000);
	char response[ACP_BUF_MAX_LENGTH];
	memset(response, 0, sizeof response);
	switch(acpserial_readResponse(self->fd, response, ACP_BUF_MAX_LENGTH)){
		case ACP_SUCCESS: 
			break;
		case ACP_ERROR_DEVICE_DISCONNECTED:
			gotoOpen(self);
			return;
		default: return;
	}
	thread_cancelDisable();
	acpsygc_setData(gcommand, response, ACP_BUF_MAX_LENGTH);
	thread_cancelEnable();
}

#ifdef MODE_DEBUG
static void cleanup_handler(void *arg) {
	AcpsyPort *self = arg;
	printf("cleaning up serial port thread (fd=%s)\n", self->param.filename);
}
#endif

static void *thread_main(void *arg) {
	AcpsyPort *self = arg;
	printdo("thread with fd=%d has been started\n", self->fd);
#ifdef MODE_DEBUG
    pthread_cleanup_push(cleanup_handler, self);
#endif
	while(1) {
		struct timespec t1 = getCurrentTime();
		CONTROL(self);
		delayTsIdleRest(self->cycle_duration, t1);
	}
#ifdef MODE_DEBUG
	pthread_cleanup_pop(1);
#endif
}

//AcpsyPort *acpsyp_new() {
	//size_t sz = sizeof (AcpsyPort);
	//AcpsyPort *self = malloc(sz);
	//if(self == NULL) {
		//putsde("failed to allocate memory for new AcpsyPort\n");
		//return self;
	//}
	//memset(self, 0, sz);
	//return self;
//}

//AcpsyPort *acpsyp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_dps, AcpsyIDLListm *ids){
	//AcpsyPort *self = acpsyp_new();
	//if(self == NULL){
		//return NULL;
	//}
	//self->fd = -1;
	//self->next = NULL;
	//self->ids = ids;
	//self->cycle_duration = ACPSYP_CYCLE_DURATION_TRY_OPEN;
	//self->control = step_TRY_OPEN;
	//if(!acpspParam_set(&self->param, serial_file_name, serial_rate, serial_dps)){
		//printde("bad serial param:%s %s at rate %d\n", serial_file_name, serial_dps, serial_rate);
		//free(self);
		//return NULL;
	//}
	//if(!thread_create(&self->thread, thread_main, self)) {
		//free(self);
		//return NULL;
	//}
	//return self;
//}

//void acpsyp_free(AcpsyPort *self) {
	//STOP_THREAD(self->thread)
	//close(self->fd);
	//acpspParam_free(&self->param);
	//free(self);
//}

int acpsyp_begin(AcpsyPort *self, const char *serial_file_name, int serial_rate, const char *serial_dps, AcpsyIDList *ids){
	memset(self, 0, sizeof *self);
	self->fd = -1;
	self->ids = ids;
	self->cycle_duration = ACPSYP_CYCLE_DURATION_TRY_OPEN;
	self->control = step_TRY_OPEN;
	if(!acpspParam_set(&self->param, serial_file_name, serial_rate, serial_dps)){
		printde("bad serial param:%s %s at rate %d\n", serial_file_name, serial_dps, serial_rate);
		return 0;
	}
	if(!thread_create(&self->thread, thread_main, self)) {
		return 0;
	}
	return 1;
}

void acpsyp_free(AcpsyPort *self) {
	STOP_THREAD(self->thread)
	close(self->fd);
	acpspParam_free(&self->param);
}
