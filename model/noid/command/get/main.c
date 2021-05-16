#include "main.h"

void ngc_free(NoidGetCommand *self){
	mutex_free(&self->mutex);
}

void ngc_sendDataToClient(NoidGetCommand *self, int tcp_fd ) {
	if(self->result == ACP_SUCCESS ){
		char q[ACP_BUF_MAX_LENGTH];
		mutex_lock(&self->mutex);
		strncpy(q, self->data, ACP_BUF_MAX_LENGTH);
		mutex_unlock(&self->mutex);
		acptcp_send(tcp_fd, q);
	}else{
		acptcp_send(tcp_fd, ACP_EMPTY_PACK_STR);
	}
}

void ngc_setData(NoidGetCommand *self, const char *data, size_t data_len, int result){
	mutex_lock(&self->mutex);
	strncpy(self->data, data, data_len);
	self->result = result;
	printdo("\tsaving result: %s, %d\n", self->data, self->result);
	mutex_unlock(&self->mutex);
}
