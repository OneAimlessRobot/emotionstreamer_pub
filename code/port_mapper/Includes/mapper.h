#ifndef MAPPER_H
#define MAPPER_H

#define PORT_MAPPER_JOIN_STRING "hey!_lemme_in!!!!"
#define PORT_MAPPER_LEAVE_STRING "see_you_lateeer!!!"
#define PORT_MAPPER_RESERVE_STRING "can_I_make_a_reservation?"
#define PORT_MAPPER_UNRESERVE_STRING "It_was_a_pleasure!"
#define NUM_PORTS_TO_GIVE 2
typedef enum command_char{CHECK_PORT='c',
				PRINT_STUFF='p',
				STOP_MAPPER='s',
				PRINT_HELP='h'}command_char;
typedef enum port_state{PORT_RESERVED=-1,
				PORT_FREE=0,
				PORT_ALLOCATED=1}port_state;

typedef struct port_mapper{

	
	int32_t* port_arr; //its a 32 bit int because it makes using my mtx functions more convenient
	int32_t curr_num_ports;
	int running;
	int socket;
	struct sockaddr_in addr_struct;
	fd_set con_fds;
}port_mapper;


void port_mapper_init(ip_cache_entry* ent);


#endif
