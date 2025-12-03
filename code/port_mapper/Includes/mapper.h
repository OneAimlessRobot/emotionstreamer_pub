#ifndef MAPPER_H
#define MAPPER_H

#define PORT_MAPPER_AWKWARD_JOIN_STRING "c-can_i_j-...j--_erm--"
#define PORT_MAPPER_AWKWARD_LEAVE_STRING "ok--_okay,_bye!"
#define PORT_MAPPER_JOIN_STRING "hey!_lemme_in!!!!"
#define PORT_MAPPER_JOIN_DID_YOU_GET_IT_STRING "Broooooooooo!!!!_be_welcooome!!!"
#define PORT_MAPPER_JOIN_GOT_IT_STRING "\"thx_FRIENNND!!!!!!!\""
#define PORT_MAPPER_LEAVE_STRING "see_you_lateeer!!!"
#define PORT_MAPPER_RESERVE_STRING "can_I_make_a_reservation?"
#define PORT_MAPPER_UNRESERVE_STRING "It_was_a_pleasure!"
#define PORT_MAPPER_RESERVE_DID_YOU_GET_IT_STRING "\"It_will_be_a_pleasure_to_have_you_and_we_hope_to_help_you_feel_at_home\""
#define PORT_MAPPER_RESERVE_GOT_IT_STRING "\"Thanks_for_having_me,_sir.\""
#define NUM_PORTS_TO_REMOVE (DEF_DATASIZE)
#define NUM_PORTS_TO_GIVE 9

typedef enum command_char{CHECK_PORT='c',
				PRINT_STUFF='p',
				STOP_MAPPER='s',
				PRINT_HELP='h',
				TERM_SIGNAL=3}command_char;
typedef enum port_state{PORT_RESERVED=-1,
				PORT_FREE=0,
				PORT_ALLOCATED=1}port_state;

typedef struct port_mapper{

	
	int32_t* port_arr; //its a 32 bit int because it makes using my mtx functions more convenient
	uint16_t curr_num_ports;
	int socket;
	struct sockaddr_in addr_struct;
	fd_set con_fds;
}port_mapper;


void port_mapper_init(ip_cache_entry* ent);


#endif
