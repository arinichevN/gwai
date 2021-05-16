#include "main.h"

static void step_RUN(AcpscPort *self);
static void step_IDLE(AcpscPort *self);
static void step_TRY_OPEN(AcpscPort *self);
static void step_SEARCH_IDS(AcpscPort *self);

static void gotoOpen(AcpscPort *self){
	close(self->fd);
	self->cycle_duration = ACPSCP_CYCLE_DURATION_TRY_OPEN;
	self->control = step_TRY_OPEN;
	printdo("port %s disconnected, trying to open it again...\n", self->param.filename);
}

static void disconnectIDs(AcpscPort *self){
	FOREACH_LLIST(id, self->ids, AcpscID){
		acpscid_lock(id);
		if(id->owner == self){
			id->state = ACPSCID_PORT_DISCONNECTED;
		}
		acpscid_unlock(id);
	}
}

static void step_TRY_OPEN(AcpscPort *self){
	//thread_cancelDisable();
	mutex_lock(&self->mutex);
	if (!serial_init(&self->fd, self->param.filename, self->param.rate, self->param.dps)) {
		printde("failed to open serial: %s %d %s\n", self->param.filename, self->param.rate, self->param.dps);
		mutex_unlock(&self->mutex);
		//thread_cancelEnable();
		return;
	}
	printdo("serial opened: %s %d %s\n", self->param.filename, self->param.rate, self->param.dps);
	self->cycle_duration = ACPSCP_CYCLE_DURATION_SEARCH;
	self->control = step_SEARCH_IDS;
	mutex_unlock(&self->mutex);
	//thread_cancelEnable();
}

#define RQLEN 40
#define RSLEN 40
static void step_SEARCH_IDS(AcpscPort *self){
	int ids_not_found_count = 0;
	FOREACH_LLIST(id, self->ids, AcpscID){
		acpscid_lock(id);
		int need_search = acpscid_needSearchOnPort(id, self);
		acpscid_unlock(id);
		if(!need_search){
			continue;
		}
		//thread_cancelDisable();
		mutex_lock(&self->mutex);
		int remote_id = id->value;
		char request_str[RQLEN];
		memset(request_str, 0, sizeof request_str);
		acpserial_buildPackII(request_str, RQLEN, ACP_SIGN_REQUEST_GET, CMD_NOID_GET_EXISTS, remote_id);
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
		int rid, exs = 0;
	    r = acpserial_extractI2(response_str, RSLEN, &rid, &exs);
	    if(r != ACP_SUCCESS){
			printde("failed to parse response for ID %d\n", remote_id);
			goto failed;
		}
		if(exs != ACP_CHANNEL_EXISTS){
			printde("id %d returned not exists\n", remote_id);
			goto failed;
		}
	    if(rid != remote_id){
			printde("expected %d id but returned %d\n", remote_id, rid);
			goto failed;
		}
		printdo("id %d found on %s\n", remote_id, self->param.filename);
		acpscid_lock(id);
		if(!acpscid_setOwner(id, self)){
			;
		}
		acpscid_unlock(id);
		mutex_unlock(&self->mutex);
		//thread_cancelEnable();
		NANOSLEEP(0, 100000);
		continue;
		failed:
		printdo("id %d not found on %s\n", remote_id, self->param.filename);
		ids_not_found_count++;
		mutex_unlock(&self->mutex);
		//thread_cancelEnable();
		NANOSLEEP(0, 100000);
		continue;
		disconnected:
		disconnectIDs(self);
		gotoOpen(self);
		mutex_unlock(&self->mutex);
		//thread_cancelEnable();
		return;
	}
	if(ids_not_found_count == 0){
		ton_reset(&self->jump_to_idle_tmr);
		self->cycle_duration = ACPSCP_CYCLE_DURATION_RUN;
		self->control = step_RUN;
	}
	
}
#undef RQLEN
#undef RSLEN

static void touch(AcpscPort *self){
	ton_reset(&self->jump_to_idle_tmr);
	if(self->control == step_IDLE){
		self->cycle_duration = ACPSCP_CYCLE_DURATION_RUN;
		self->control = step_RUN;
	}
}

static void step_RUN(AcpscPort *self){
	if(ton(&self->jump_to_idle_tmr)){
		mutex_lock(&self->mutex);
		self->cycle_duration = ACPSCP_CYCLE_DURATION_IDLE;
		self->control = step_IDLE;
		mutex_unlock(&self->mutex);
	}
}

static void step_IDLE(AcpscPort *self){
	//thread_cancelDisable();
	mutex_lock(&self->mutex);
	if(!serial_alive(self->fd)){
		gotoOpen(self);
	};
	mutex_unlock(&self->mutex);
	//thread_cancelEnable();
}



#ifdef MODE_DEBUG
static void cleanup_handler(void *arg) {
	AcpscPort *self = arg;
	printf("cleaning up serial port thread (fd=%s)\n", self->param.filename);
}
#endif

static void *thread_main(void *arg) {
	AcpscPort *self = arg;
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

AcpscPort *acpscp_new() {
	size_t sz = sizeof (AcpscPort);
	AcpscPort *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new AcpscPort\n");
		return self;
	}
	memset(self, 0, sz);
	return self;
}

AcpscPort *acpscp_newBegin(const char *serial_file_name, int serial_rate, const char *serial_dps, AcpscIDLListm *ids){
	AcpscPort *self = acpscp_new();
	if(self == NULL){
		return NULL;
	}
	self->fd = -1;
	self->next = NULL;
	self->ids = ids;
	ton_setInterval(ACPSCP_WAIT_IDLE_INTERVAL, &self->jump_to_idle_tmr);
	self->cycle_duration = ACPSCP_CYCLE_DURATION_TRY_OPEN;
	self->control = step_TRY_OPEN;
	if(!acpspParam_set(&self->param, serial_file_name, serial_rate, serial_dps)){
		printde("bad serial param:%s %s at rate %d\n", serial_file_name, serial_dps, serial_rate);
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

void acpscp_terminate(AcpscPort *self){
	mutex_lock(&self->mutex);
	STOP_THREAD(self->thread)
	mutex_unlock(&self->mutex);
}

void acpscp_free(AcpscPort *self) {
	close(self->fd);
	acpspParam_free(&self->param);
	mutex_free(&self->mutex);
	free(self);
}

static int isActive(AcpscPort *self){
	if(self->control == step_RUN || self->control == step_IDLE || self->control == step_SEARCH_IDS) return 1;
	return 0;
}

void acpscp_lock(AcpscPort *self){
	mutex_lock(&self->mutex);
}

void acpscp_unlock(AcpscPort *self){
	mutex_unlock(&self->mutex);
}

//lock object before using this function, and unlock after
int acpscp_send(AcpscPort *self, const char *request_str){
	if(!isActive(self)){
		return ACP_ERROR_DEVICE_DISCONNECTED;
	}
	touch(self);
	int r = acpserial_send(self->fd, request_str);
	if(r == ACP_ERROR_DEVICE_DISCONNECTED){
		gotoOpen(self);
	}
	return r;
}

//lock object before using this function, and unlock after
int acpscp_readResponse(AcpscPort *self, char *buf, size_t buf_len){
	if(!isActive(self)){
		return ACP_ERROR_DEVICE_DISCONNECTED;
	}
	touch(self);
	int r = acpserial_readResponse(self->fd, buf, buf_len);
	if(r == ACP_ERROR_DEVICE_DISCONNECTED){
		gotoOpen(self);
	}
	return r;
}
