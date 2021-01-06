#include "ACPTS.h"

int acpts_startNewConnection(ACPTS *self){
	ACPTSConnectionLList *list = &self->connections;
	if(list->length >= self->conn_num_max){
		printde("list->length = %zu, conn_num_max = %zu\n", list->length, self->conn_num_max );
		return 0;
	}
	ACPTSConnection *nconnection = acptsconn_new(list->length, self->serve_function);
	if(nconnection == NULL){
		return 0;
	}
	if(!acptsconnList_push(list, nconnection)){
		acptsconn_free(nconnection);
		putsde("failed to add new connection to list\n");
		return 0;
	}
	printdo("TCP CONNECTION SERVICE THREAD STARTED %zu\n", nconnection->id);
	return 1;
}

void acpts_accept(ACPTS *self){
    int fd_conn = accept(self->fd, (struct sockaddr*)NULL ,NULL);
	ACPTSConnectionLList *list = &self->connections;
	FOREACH_LLIST(curr, list, ACPTSConnection) {
		if(acptsconn_wakeUp(curr, fd_conn)){
			return;
		}
	}
	putsdo("acpts_accept: no free service threads, trying to create new service thread\n");
	if(!acpts_startNewConnection(self)){
		putsdo("acpts_accept: connection rejected\n");
		close(fd_conn);
	}
}

static void *thread_main(void *arg){
	ACPTS *self = arg;
	putsdo("hello from parallel server accept thread\n");
	while(1){
		acpts_accept(self);
	}
	return NULL;
}

void acpts_free(ACPTS *self){
	if (self == NULL) return;
	
	STOP_THREAD(self->thread)
	close(self->fd);
	acptsconnList_free(&self->connections);
	free(self);
}

ACPTS *acpts_newBegin(int port, size_t conn_num_max, ACPTSServeFunction serve_function) {
	if(conn_num_max < 1){
		putsde("no connections required\n");
		return NULL;
	}
	size_t sz = sizeof (ACPTS);
	ACPTS *self = malloc(sz);
	if(self == NULL) {
		putsde("failed to allocate memory for new ACPTS\n");
		return self;
	}
	memset(self, 0, sz);
	self->fd = -1;
	self->conn_num_max = conn_num_max;
	self->serve_function = serve_function;
	if(!acptcp_initServer(&self->fd, port)) {
		putsde("failed to initialize server socket\n");
		free(self);
		return NULL;
	}
	LLIST_RESET(&self->connections);
	if(!acpts_startNewConnection(self)){
		putsde("failed to start connection service thread\n");
		acptsconnList_free(&self->connections);
		free(self);
		return NULL;
	}
	if(!thread_create(&self->thread, thread_main, self)) {
		putsde("failed to create server main thread\n");
		acptsconnList_free(&self->connections);
		free(self);
		return NULL;
	}
	return self;
}
