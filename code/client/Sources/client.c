#include "../Includes/preprocessor.h"
#include <ncurses.h>
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
#include "../Includes/download_func.h"

static struct sockaddr_in server_ip_address;

static con_t client_con_obj;

static void sigint_handler(int signal){

	close_con(&client_con_obj);
	//endwin();
	fclose(logstream);
	exit(-1);
}
static void sigpipe_handler(int signal){

	sigint_handler(signal);

}
static int down_file_size(void){

		int down_size=-1;
		clear_con_data(&client_con_obj);
		con_read_udp(&client_con_obj,client_data_times_pair);
		sscanf((char*)client_con_obj.udp_data,"%d",&down_size);
		if(down_size<=0){

			char* reason= down_size ? UNSUCESSFUL_DOWNLOAD_NOFILE :UNSUCESSFUL_DOWNLOAD_CON_ERROR;
			printf(UNSUCESSFUL_DOWNLOAD,reason);
			raise(SIGINT);
		}
		clear_con_data(&client_con_obj);
		snprintf((char*)client_con_obj.ack_udp_data,cfg_datasize,"%s",CON_STRING);
		con_send_udp_ack(&client_con_obj,client_data_times_pair);
		printf(CONTENT_DOWNLOAD_INCOMMING,down_size);
		clear_con_data(&client_con_obj);
		return down_size;

}
static void play_func(void){
		unsigned char stream_cache_data[cfg_chunk_size*cfg_stream_cache_size_chunks];
		int down_size=down_file_size();
		printf(STREAM_INCOMMING,down_size);
		player_init_stream(&client_con_obj, stream_cache_data,PLAY_PA);


}
static void down_func(char* file_name){

		int down_size=down_file_size();
		int fp=-1;
		char file_path[PATHSIZE*3-1]={0};
		snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,EXTENSION);
		if((fp=creat(file_path,0777))<0){
				perror("Nao foi possivel transferir ficheiro!!!!\n");
                                raise(SIGINT);
                }
		initscr();
		downloadtofd(client_con_obj.sockfd_tcp,fp,down_size,client_data_times_pair);
		endwin();
		printf("A musica foi guardada em: %s\n",file_path);
		raise(SIGINT);
		

}
static void peek_func(void){

		
		int down_size=down_file_size();
		printf(CONTENT_PEEK_INCOMMING);
		readalltofd(client_con_obj.sockfd_tcp,1,down_size,client_data_times_pair);
		raise(SIGINT);
		

}


//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,uint32_t tcp_s_port, char* s_hostname){
	
	unsigned char tcp_data[cfg_datasize+1];
	unsigned char udp_data[cfg_datasize+1];
	unsigned char ack_udp_data[cfg_datasize+1];
	
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
		play_func();
		break;
	case DOWN:
		down_func(file_name);
		break;
	case PEEK:
		peek_func();
		break;
	default:
		printf(UNKNOWN_REQ);
		raise(SIGINT);
		break;
	}
	return 0;
}

