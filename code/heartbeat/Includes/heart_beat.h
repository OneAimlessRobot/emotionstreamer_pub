#ifndef HEART_BEAT_H
#define HEART_BEAT_H


#define MAX_SERVERS 1000
#define HB_SERVER_NAME_SIZE 150
#define DB_FILE "./serverdb/servers.db"

typedef struct hb_acceptor{

	int is_on;
	fd_set mainfds;
	int accept_sockfd;
	char* name;
	struct con_t master_con;

}hb_acceptor;





void start_heart_beats(ip_cache_entry* ent_this,ip_cache_entry* ent_upper);




#endif

