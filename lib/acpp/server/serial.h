

#define SERVERS_STOP server_stop:close(fd_conn);
#define SERVERS_ACCEPT int fd_conn = accept ( server_fd, ( struct sockaddr* ) NULL ,NULL );


