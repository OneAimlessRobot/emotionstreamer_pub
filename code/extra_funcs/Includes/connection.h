#ifndef CONNECTION_H
#define CONNECTION_H

typedef enum con_type{SERVER_C,CLIENT_C} con_type;


#define CONNECTION_ESTABLISHED_MSG "Conectado a %s!!!!!!\n"


typedef struct con_t{

	 struct sockaddr_in
			peer_udp_addr,
			this_udp_addr,
			peer_udp_ack_addr,
			this_udp_ack_addr,
         		this_tcp_addr,
			peer_tcp_addr;
       	 int is_on,
         sockfd_udp,
         ack_sockfd_udp,
         sockfd_tcp;
	 uint8_t udp_data[DEF_DATASIZE+1];
	 uint8_t tcp_data[DEF_DATASIZE+1];
	 uint8_t ack_udp_data[DEF_DATASIZE+1];
	 con_type type;
}con_t;
//sockfd_tcp tem de ser obtida com connect ou accept!!!!!!!

//buff_triple COMES FROM THE OUTSIDE!

//DOES NOT FREE OR INITIALIZE THE BUFFS YOU GIVE IT!

void init_con(con_t* con_obj, int sockfd_tcp,con_type type);

void close_con(con_t* con_obj);

int con_send_tcp(con_t* con_obj,int_pair pair);

int con_send_udp(con_t* con_obj,int_pair pair);

int con_read_tcp(con_t* con_obj,int_pair pair);

int con_read_udp(con_t* con_obj,int_pair pair);

int con_read_udp_ack(con_t* con_obj, int_pair pair);

int con_send_udp_ack(con_t* con_obj, int_pair pair);

//initialize con_obj first!!!
void clear_con_data(con_t* con_obj);

void drop_peer_con(con_t* con_obj);

void greet(con_t*con_obj,int_pair times_pair, uint16_t curr_port);
#endif
