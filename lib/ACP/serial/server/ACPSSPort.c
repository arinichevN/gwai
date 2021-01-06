#include "ACPSSPort.h"

static void step_RUN(ACPSSPort *self);
static void step_TRY_OPEN(ACPSSPort *self);

static void step_RUN(ACPSSPort *self){
	char buf[ACP_BUF_MAX_LENGTH];
	memset(buf, 0, sizeof buf);
	switch(acpserial_readRequest(self->fd, buf, ACP_BUF_MAX_LENGTH)){
		case ACP_SUCCESS: break;
		case ACP_ERROR_DEVICE_DISCONNECTED:
			close(self->fd);
			self->cycle_duration = ACPSSP_CYCLE_DURATION_TRY_OPEN;
			self->control = step_TRY_OPEN;
			return;
		default: return;
	}
	char sign = buf[ACP_IND_SIGN];
	switch(sign){
		case ACP_SIGN_REQUEST_GET:				break;
		case ACP_SIGN_REQUEST_SET:				break;
		case ACP_SIGN_REQUEST_SET_BROADCAST:	break;
		case ACP_SIGN_REQUEST_GET_BROADCAST:	break;
		default:								putsde("bad sign\n"); return;
	}
	int cmd;
	if(!acp_packGetCellI(buf, ACP_REQUEST_IND_CMD, &cmd)){
		putsde("failed to get command\n");
		return;
	}
	//printdo("command: %d\n", cmd);
	for(size_t i = 0; i < self->acnodes_count; i++){
		ACPSSCommandNode *node = &self->acnodes[i];
		if(cmd == node->command){
			int r = node->func(buf);
			//putsdo("command found\n");
			if(r == ACP_SENDING_REQUIRED){
				acpserial_send(self->fd, buf);
				return;
			}
			return;
		}
		
	}
	putsde("command not found");
	
}

static void step_TRY_OPEN(ACPSSPort *self){
	if (!serial_init(&self->fd, self->param.filename, self->param.rate, self->param.config)) {
		printde("failed to initialize serial: %s %d %s\n", self->param.filename, self->param.rate, self->param.config);
		return;
	}
	printdo("serial opened: %s %d %s\n", self->param.filename, self->param.rate, self->param.config);
	self->cycle_duration = ACPSSP_CYCLE_DURATION_RUN;
	self->control = step_RUN;
}

#ifdef MODE_DEBUG
void acpssp_cleanup_handler(void *arg) {
	ACPSSPort *self = arg;
	printf("cleaning up serial port thread (fd=%s)\n", self->param.filename);
}
#endif

static void *thread_main(void *arg) {
	ACPSSPort *self = arg;
	printdo("thread with fd=%d has been started\n", self->fd);
#ifdef MODE_DEBUG
    pthread_cleanup_push(acpssp_cleanup_handler, self);
#endif
	while(1) {
		struct timespec t1 = getCurrentTime();
		thread_cancelDisable();
		CONTROL(self);
		thread_cancelEnable();
		delayTsIdleRest(self->cycle_duration, t1);
	}
#ifdef MODE_DEBUG
	pthread_cleanup_pop(1);
#endif
}

ACPSSPort *acpssp_new() {
	size_t sz = sizeof (ACPSSPort);
	ACPSSPort *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new ACPSSPort\n");
		return self;
	}
	memset(self, 0, sz);
	return self;
}

ACPSSPort *acpssp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_config, ACPSSCommandNode *acnodes, size_t acnodes_count){
	ACPSSPort *self = acpssp_new();
	if(self == NULL){
		return NULL;
	}
	self->acnodes = acnodes;
	self->acnodes_count = acnodes_count;
	self->next = NULL;
	self->cycle_duration = ACPSSP_CYCLE_DURATION_TRY_OPEN;
	self->control = step_TRY_OPEN;
	if(!acpspParam_set(&self->param, serial_file_name, serial_rate, serial_config)){
		printde("bad serial param:%s %s at rate %d\n", serial_file_name, serial_config, serial_rate);
		free(self);
		return NULL;
	}
	if(!mutex_init(&self->mutex)) {
		free(self);
		return NULL;
	}
	if(!thread_create(&self->thread, thread_main, self)) {
		mutex_free(&self->mutex);
		free(self);
		return NULL;
	}
	
	return self;
}

void acpssp_free(ACPSSPort *self) {
	mutex_lock(&self->mutex);
	STOP_THREAD(self->thread)
	close(self->fd);
	acpspParam_free(&self->param);
	mutex_unlock(&self->mutex);
	mutex_free(&self->mutex);
	free(self);
}
