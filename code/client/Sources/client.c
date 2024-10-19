#include "../Includes/preprocessor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/streamer_client.h"
#include "../Includes/client.h"

static struct sockaddr_in server_ip_address;

static con_t client_con_obj;

static void sigpipe_handler(int signal){

	exit(-1);
}
static void sigint_handler(int signal){

	exit(-1);
}

//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,uint32_t tcp_s_port, char* s_hostname){
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);

	req_type the_type= str_to_req_type(req_field);


	if(the_type==NA){
		printf(UNKNOWN_REQ);
		raise(SIGINT);
	}

	init_addr(&server_ip_address,s_hostname,tcp_s_port);

	int client_sock=client_con_obj.sockfd_tcp= socket(AF_INET,SOCK_STREAM,0);

	if(client_con_obj.sockfd_tcp<0){

		raise(SIGINT);
	}

	tryConnect(&client_sock,CLIENT_CON_TIMES_PAIR,&server_ip_address);

	init_con(&client_con_obj,client_sock,CLIENT_C);

	greet(&client_con_obj);

	snprintf((char*)client_con_obj.data,DEF_DATASIZE,"%s %s",req_field,file_name);

	con_send_tcp(&client_con_obj,CLIENT_DATA_TIMES_PAIR);

	

	switch(the_type){

	case PLAY:
		player_init_stream(&client_con_obj);
		break;
	case PEEK:
		printf(CONTENT_PEEK_INCOMMING);
		readalltofd(client_con_obj.sockfd_tcp,1,CLIENT_DATA_TIMES_PAIR);
		break;
	default:
		printf(UNKNOWN_REQ);
		break;
	}
	raise(SIGINT);
	return 0;
}

