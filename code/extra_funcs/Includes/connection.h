#ifndef CONNECTION_H
#define CONNECTION_H

typedef enum con_type{SERVER_C,CLIENT_C} con_type;


#define CONNECTION_ESTABLISHED_MSG "Conectado a %s!!!!!!\n"
#define PORT_ADVANCE_CONSTANT 5000

typedef struct con_t{

	 struct sockaddr_in
			peer_udp_addr,
			this_udp_addr,
			peer_udp_ack_addr,
			this_udp_ack_addr,
         		this_tcp_addr,
			peer_tcp_addr;
	 struct sockaddr_in port_mapper_addr;
       	 int is_on,
         sockfd_udp,
         ack_sockfd_udp,
         sockfd_tcp;
	 uint8_t udp_data[DEF_DATASIZE+1];
	 uint8_t tcp_data[DEF_DATASIZE+1];
	 uint8_t ack_udp_data[DEF_DATASIZE+1];
	 uint16_t udp_data_local_port,
		tcp_data_local_port,
		udp_ack_local_port;
	 uint16_t udp_data_peer_port,
		tcp_data_peer_port,
		udp_ack_peer_port;
	 uint16_t listen_port; //if launched within a server of some variety;
	 ip_cache_entry port_mapper_entry;
	 int app_level_proto;
	 con_type type;
}con_t;
//sockfd_tcp tem de ser obtida com connect ou accept!!!!!!!

//buff_triple COMES FROM THE OUTSIDE!

//DOES NOT FREE OR INITIALIZE THE BUFFS YOU GIVE IT!

void init_con(con_t* con_obj, int sockfd_tcp,con_type type,uint16_t listen_port,ip_cache_entry* ent,int app_level_proto);

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

void ask_for_port(uint16_t* port,ip_cache_entry* ent);

void send_ports_back(con_t* obj);

void ask_for_ports(con_t* obj);

void send_port_back(uint16_t port,ip_cache_entry* ent);

void reserve_local_listening_port(uint16_t port_to_allocate,ip_cache_entry *ent);

void unreserve_local_listening_port(uint16_t port_to_allocate,ip_cache_entry*ent);

void greet(con_t*con_obj,int_pair times_pair, int_pair holepunching_times_pair);
#endif
