#include "ACPSCPort.h"

static void step_RUN(ACPSCPort *self);

static void step_TRY_OPEN(ACPSCPort *self){
	thread_cancelDisable();
	mutex_lock(&self->mutex);
	if (!serial_init(&self->fd, self->param.filename, self->param.rate, self->param.config)) {
		printde("failed to initialize serial: %s %d %s\n", self->param.filename, self->param.rate, self->param.config);
		mutex_unlock(&self->mutex);
		thread_cancelEnable();
		return;
	}
	printdo("serial opened: %s %d %s\n", self->param.filename, self->param.rate, self->param.config);
	self->cycle_duration = ACPSCP_CYCLE_DURATION_RUN;
	self->control = step_RUN;
	self->onConnected(self->mapper, self);
	mutex_unlock(&self->mutex);
	thread_cancelEnable();
}

static void step_RUN(ACPSCPort *self){
	;
}

#define RQLEN 40
#define RSLEN 40
static void step_ID_EXISTS(ACPSCPort *self) {
	thread_cancelDisable();
	mutex_lock(&self->mutex);
	char request_str[RQLEN];
	memset(request_str, 0, sizeof request_str);
	
	acpserial_buildPackII(request_str, RQLEN, ACP_SIGN_REQUEST_GET, CMD_NOID_GET_EXISTS, self->remote_id);
	int r = acpserial_send(self->fd, request_str);
	if(r == ACP_ERROR_DEVICE_DISCONNECTED){
		goto disconnected;
	}
	if (r != ACP_SUCCESS) {
		goto failed;
	}
	NANOSLEEP(0, 100000);
	char response_str[RSLEN];
	memset(response_str, 0, sizeof response_str);
	r = acpserial_readResponse(self->fd, response_str, RSLEN);
	if(r == ACP_ERROR_DEVICE_DISCONNECTED){
		goto disconnected;
	}
	if (r != ACP_SUCCESS) {
		goto failed;
	}
	int id, exs = 0;
    r = acpserial_extractI2(response_str, RSLEN, &id, &exs);
    if(r != ACP_SUCCESS){
		printde("failed to parse response for ID %d\n", self->remote_id);
		goto failed;
	}
	if(exs != ACP_CHANNEL_EXISTS){
		printde("ID %d returned not exists\n", self->remote_id);
		goto failed;
	}
    if(id != self->remote_id){
		printde("expected %d id but returned %d\n", self->remote_id, id);
		goto failed;
	}
	self->cycle_duration = ACPSCP_CYCLE_DURATION_RUN;
	self->control = step_RUN;
	mutex_unlock(&self->mutex);
	self->onCheckRemoteIDExistsDone(self->mapper, self->remote_id, ACP_SUCCESS, self);
	thread_cancelEnable();
	return;
	failed:
	self->cycle_duration = ACPSCP_CYCLE_DURATION_RUN;
	self->control = step_RUN;
	mutex_unlock(&self->mutex);
	self->onCheckRemoteIDExistsDone(self->mapper, self->remote_id, ACP_ERROR, self);
	thread_cancelEnable();
	return;
	disconnected:
	close(self->fd);
	self->cycle_duration = ACPSCP_CYCLE_DURATION_TRY_OPEN;
	self->control = step_TRY_OPEN;
	mutex_unlock(&self->mutex);
	self->onCheckRemoteIDExistsDone(self->mapper, self->remote_id, ACP_ERROR, self);
	self->onDisconnected(self->mapper, self);
	thread_cancelEnable();
}
#undef RQLEN
#undef RSLEN

#ifdef MODE_DEBUG
static void cleanup_handler(void *arg) {
	ACPSCPort *self = arg;
	printf("cleaning up serial port thread (fd=%s)\n", self->param.filename);
}
#endif

static void *thread_main(void *arg) {
	ACPSCPort *self = arg;
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

ACPSCPort *acpscp_new() {
	size_t sz = sizeof (ACPSCPort);
	ACPSCPort *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new ACPSCPort\n");
		return self;
	}
	memset(self, 0, sz);
	return self;
}

ACPSCPort *acpscp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_config){
	ACPSCPort *self = acpscp_new();
	if(self == NULL){
		return NULL;
	}
	self->fd = -1;
	self->next = NULL;
	self->mapper = NULL;
	self->onCheckRemoteIDExistsDone = NULL;
	self->onConnected = NULL;
	self->onDisconnected = NULL;
	self->cycle_duration = ACPSCP_CYCLE_DURATION_TRY_OPEN;
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

void acpscp_free(ACPSCPort *self) {
	mutex_lock(&self->mutex);
	STOP_THREAD(self->thread)
	close(self->fd);
	acpspParam_free(&self->param);
	mutex_unlock(&self->mutex);
	mutex_free(&self->mutex);
	free(self);
}

static int isActive(ACPSCPort *self){
	if(self->control == step_RUN) return 1;
	return 0;
}

void acpscp_lock(ACPSCPort *self){
	mutex_lock(&self->mutex);
}

void acpscp_unlock(ACPSCPort *self){
	mutex_unlock(&self->mutex);
}

//lock object before using this function, and unlock after
int acpscp_send(ACPSCPort *self, const char *request_str){
	if(!isActive(self)){
		return ACP_ERROR_DEVICE_DISCONNECTED;
	}
	int r = acpserial_send(self->fd, request_str);
	if(r == ACP_ERROR_DEVICE_DISCONNECTED){
		close(self->fd);
		self->cycle_duration = ACPSCP_CYCLE_DURATION_TRY_OPEN;
		self->control = step_TRY_OPEN;
		self->onDisconnected(self->mapper, self);
	}
	return r;
}

//lock object before using this function, and unlock after
int acpscp_readResponse(ACPSCPort *self, char *buf, size_t buf_len){
	if(!isActive(self)){
		return ACP_ERROR_DEVICE_DISCONNECTED;
	}
	int r = acpserial_readResponse(self->fd, buf, buf_len);
	if(r == ACP_ERROR_DEVICE_DISCONNECTED){
		close(self->fd);
		self->cycle_duration = ACPSCP_CYCLE_DURATION_TRY_OPEN;
		self->control = step_TRY_OPEN;
		self->onDisconnected(self->mapper, self);
	}
	return r;
}

void acpscp_checkRemoteIDExists(ACPSCPort *self, int id){
	if(!isActive(self)){
		return;
	}
	mutex_lock(&self->mutex);
	self->remote_id = id;
	self->cycle_duration = ACPSCP_CYCLE_DURATION_ID_EXISTS;
	self->control = step_ID_EXISTS;
	mutex_unlock(&self->mutex);
}
