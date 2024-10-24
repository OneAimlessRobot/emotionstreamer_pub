#include "../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/configs.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/ripped_code.h"
#include "../Includes/streamer_client.h"
#include "../Includes/client.h"

static struct sockaddr_in server_ip_address;

static con_t client_con_obj;
int fp=-1;
static void sigpipe_handler(int signal){

	close_con(&client_con_obj);
	exit(-1);
}
static void sigint_handler(int signal){

	close_con(&client_con_obj);
	exit(-1);
}

//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,uint32_t tcp_s_port, char* s_hostname){
	
	unsigned char tcp_data[cfg_datasize+1];
	unsigned char udp_data[cfg_datasize+1];
	unsigned char ack_udp_data[cfg_datasize+1];
	unsigned char stream_cache_data[cfg_chunk_size*cfg_stream_cache_size_chunks];

	buff_triple con_buffs={NULL};
        con_buffs[0]=tcp_data;
        con_buffs[1]=udp_data;
	con_buffs[2]=ack_udp_data;
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

	tryConnect(&client_sock,client_con_times_pair,&server_ip_address);

	init_con(&client_con_obj,client_sock,CLIENT_C,con_buffs);

	getsockname(client_con_obj.sockfd_tcp,(struct sockaddr*)&client_con_obj.this_tcp_addr,socklenvar);
	greet(&client_con_obj,client_con_times_pair,client_con_obj.this_tcp_addr.sin_port);

	snprintf((char*)client_con_obj.tcp_data,cfg_datasize,"%s %s",req_field,file_name);

	con_send_tcp(&client_con_obj,client_data_times_pair);

	

	switch(the_type){

	case PLAY:
		player_init_stream(&client_con_obj, stream_cache_data,PLAY_PA);
		break;
	case DOWN:
		printf(CONTENT_DOWNLOAD_INCOMMING);
		char file_path[PATHSIZE*3-1]={0};
		snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,EXTENSION);
		printf("A file_path é: %s\n",file_path);
                if((fp=creat(file_path,0777))<0){
				perror("Nao foi possivel transferir ficheiro!!!!\n");
                                raise(SIGINT);
                }
		readalltofd(client_con_obj.sockfd_tcp,fp,client_data_times_pair);
		raise(SIGINT);
		break;
	case PEEK:
		printf(CONTENT_PEEK_INCOMMING);
		readalltofd(client_con_obj.sockfd_tcp,1,client_data_times_pair);
		raise(SIGINT);
		break;
	default:
		printf(UNKNOWN_REQ);
		raise(SIGINT);
		break;
	}
	return 0;
}

