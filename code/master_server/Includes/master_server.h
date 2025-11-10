#ifndef MASTER_SERVER_H
#define MASTER_SERVER_H


#define MAX_HB_SERVERS 1000
#define DB_FILE "./serverdb/servers.db"

typedef struct master_acceptor{

	int is_on;
	fd_set mainfds;
	int accept_sockfd;
	struct sockaddr_in tcp_addr;

}master_acceptor;





void start_master(char* hostname, uint16_t port);





#endif
