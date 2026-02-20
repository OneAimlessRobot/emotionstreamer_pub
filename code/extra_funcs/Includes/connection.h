#ifndef CONNECTION_H
#define CONNECTION_H

typedef enum con_type{SERVER_C,CLIENT_C} con_type;

#define CONNECTION_ESTABLISHED_MSG "Conectado a %s!!!!!!\n"
#define PORT_ADVANCE_CONSTANT 5000

typedef struct con_t{

	struct sockaddr_in
		this_tcp_addr,
		peer_tcp_addr;
	struct sockaddr_in port_mapper_addr;
	int is_on,
	sockfd_tcp;
	uint8_t tcp_data[DEF_DATASIZE+1];
	ip_cache_entry port_mapper_entry;
	con_type type;
	uint8_t is_ssl;
	SSL* con_ssl;
}con_t;
//sockfd_tcp tem de ser obtida com connect ou accept!!!!!!!

//buff_triple COMES FROM THE OUTSIDE!

//DOES NOT FREE OR INITIALIZE THE BUFFS YOU GIVE IT!

void init_con(con_t* con_obj, int sockfd_tcp,con_type type,ip_cache_entry* ent,uint8_t is_ssl);

void close_con(con_t* con_obj, int RIGHT_NOW,int close_for_good);

void close_udp_of_con(con_t* con_obj);

int con_send_tcp(con_t* con_obj,int_pair pair);

int con_read_tcp(con_t* con_obj,int_pair pair);

//initialize con_obj first!!!
void clear_con_data(con_t* con_obj);

void ask_for_port(uint16_t* port,ip_cache_entry* ent);

void send_port_back(uint16_t port,ip_cache_entry* ent);

void ask_for_ports(ip_cache_entry* ent);

void send_ports_back(ip_cache_entry* ent,uint16_t port_that_works);

void free_attempted_ports(uint16_t port_that_works,ip_cache_entry* ent);
void connection_attempt_circuit(int* socket_fd,void (*quit_handler)(int, void*),struct sockaddr_in* src_address,
				struct sockaddr_in* dst_address,
					ip_cache_entry* src_ent,ip_cache_entry* port_mapper_ent,
					int_pair con_times_pair,
					void*ptr);

void greet(con_t*con_obj,int_pair times_pair);
#endif
