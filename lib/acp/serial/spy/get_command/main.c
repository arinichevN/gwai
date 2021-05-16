#include "main.h"

//AcpsyGetCommand *acpsygc_newBegin(int command, struct timespec timeout){
	//size_t sz = sizeof (AcpsyGetCommand);
	//AcpsyGetCommand *self = malloc(sz);
	//if(self == NULL) {
		//putsde("failed to allocate memory for new AcpsyGetCommand\n");
		//return self;
	//}
	//memset(self, 0, sz);
	//if(!mutex_init(&self->mutex)) {
		//free(self);
		//return NULL;
	//}
	//self->value = command;
	//self->timeout = timeout;
	//self->write_time = (struct timespec) {0UL, 0UL};
	//self->next = NULL;
	//return self;
//}

//void acpsygc_free(AcpsyGetCommand *self){
	//mutex_free(&self->mutex);
	//free(self);
//}

int acpsygc_begin(AcpsyGetCommand *self, int command, struct timespec timeout){
	memset(self, 0, sizeof *self);
	if(!mutex_init(&self->mutex)) {
		return 0;
	}
	self->value = command;
	self->timeout = timeout;
	self->write_time = (struct timespec) {0UL, 0UL};
	printdo("\tcommand %d begin\n", command);
	return 1;
}

void acpsygc_free(AcpsyGetCommand *self){
	mutex_free(&self->mutex);
}

int acpsygc_getData(AcpsyGetCommand *self, int tcp_fd, char *buf, size_t data_len) {
	struct timespec now = getCurrentTime();
	if(!timeHasPassed(self->timeout, self->write_time, now)){
		mutex_lock(&self->mutex);
		strncpy(buf, self->data, data_len);
		mutex_unlock(&self->mutex);
		return 1;
	}
	return 0;
}

void acpsygc_setData(AcpsyGetCommand *self, const char *data, size_t data_len){
	mutex_lock(&self->mutex);
	strncpy(self->data, data, ACP_BUF_MAX_LENGTH);
	self->write_time = getCurrentTime();
	printdo("\tintercepted: %s\n", self->data);
	mutex_unlock(&self->mutex);
}

