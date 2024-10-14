#ifndef ENGINE_H
#define ENGINE_H

typedef struct server_state{

	char address_str[INET_ADDRSTRLEN];
        struct sockaddr_in server_udp_addr;
	struct sockaddr_in client_udp_addr;
        long int server_is_on,udp_s_port;
        int server_sock_udp;
   	int client_sock_udp;
   	fd_set rdfds;
	u_int64_t dataSize;
	/*u_int64_t totalSent;
        u_int64_t timeActive;
        double trafficRate;
	*/
}server_state;


//Strings null terminated
void serverInit(char* addr,uint32_t udp_s_port);



#endif
