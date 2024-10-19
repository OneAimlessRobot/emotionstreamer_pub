#ifndef CONNECTION_H
#define CONNECTION_H

typedef enum con_type{SERVER_C,CLIENT_C} con_type;


#define CONNECTION_ESTABLISHED_MSG "Conectado a %s!!!!!!\n"


typedef struct con_t{

	 struct sockaddr_in
			peer_tcp_addr,
			peer_udp_addr,
			this_tcp_addr,
			this_udp_addr;
         int is_on,
         sockfd_tcp,
         sockfd_udp;
	 uint8_t data[DEF_DATASIZE+1];
	 con_type type;
}con_t;
//sockfd_tcp tem de ser obtida com connect ou accept!!!!!!!


void init_con(con_t* con_obj, int sockfd_tcp, con_type type);

void close_con(con_t* con_obj);

int con_send_tcp(con_t* con_obj,int_pair pair);

int con_send_udp(con_t* con_obj,int_pair pair);

int con_read_tcp(con_t* con_obj,int_pair pair);

int con_read_udp(con_t* con_obj,int_pair pair);

//initialize con_obj first!!!
void clear_con_data(con_t* con_obj);

void greet(con_t*con_obj);
#endif
