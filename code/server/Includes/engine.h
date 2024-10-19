#ifndef ENGINE_H
#define ENGINE_H

typedef struct server_state{

	struct sockaddr_in server_tcp_addr;
        long int server_is_on,tcp_s_port;
        int server_sock_tcp;
   	fd_set rdfds;
}server_state;


//Strings null terminated
void serverInit(char* addr,uint32_t tcp_s_port);



#endif
