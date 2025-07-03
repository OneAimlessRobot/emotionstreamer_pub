#ifndef ENGINE_H
#define ENGINE_H
#define SERVER_NAME_SIZE 20
typedef struct server_state{

	char* name;
	struct sockaddr_in server_tcp_addr;
        int server_is_on;
	int server_sock_tcp;
   	fd_set rdfds;
	struct con_t hb_con;

}server_state;


//Strings null terminated
int serverInit(ip_cache_entry* ent_this,ip_cache_entry* ent_upper);



#endif
