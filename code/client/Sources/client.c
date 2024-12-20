#include "../Includes/preprocessor.h"
#include <ncurses.h>
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/chunk_player.h"
#include "../Includes/streamer_client.h"
#include "../Includes/client.h"
#include "../Includes/download_func.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/ip_cache_file_ops.h"

static struct sockaddr_in server_ip_address;
ip_cache_t cache=(ip_cache_t){NULL,0};
static con_t client_con_obj;
static method play_way=PLAY_PA;
static void sigint_handler(int signal){

	close_con(&client_con_obj);
	endwin();
	fclose(logstream);
	exit(signal);
}
static void sigpipe_handler(int signal){

	sigint_handler(signal);

}
static int down_file_size(void){

		int down_size=-1;
		clear_con_data(&client_con_obj);
		printf("Recebendo tamanho!!!\n");
		con_read_udp(&client_con_obj,client_data_times_pair);
		sscanf((char*)client_con_obj.udp_data,"%d",&down_size);
		if(down_size<=0){

			char* reason= down_size ? UNSUCESSFUL_DOWNLOAD_NOFILE :UNSUCESSFUL_DOWNLOAD_CON_ERROR;
			printf(UNSUCESSFUL_DOWNLOAD,reason);
			raise(SIGINT);
		}
		clear_con_data(&client_con_obj);
		snprintf((char*)client_con_obj.ack_udp_data,DEF_DATASIZE,"%s",CON_STRING);
		con_send_udp_ack(&client_con_obj,client_data_times_pair);
		printf(CONTENT_DOWNLOAD_INCOMMING,down_size);
		clear_con_data(&client_con_obj);
		return down_size;

}
static void play_func(void){
		int down_size=down_file_size();
		uint16_t chunk_size=0;
		printf(STREAM_INCOMMING,down_size);
		con_read_udp(&client_con_obj,client_data_times_pair);
		sscanf((char*)client_con_obj.udp_data,"%hu",&chunk_size);
		player_init_stream(&client_con_obj,chunk_size,play_way);
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
static void conf_func(void){

		int down_size=down_file_size();
		printf(CONTENT_PEEK_INCOMMING);
		readalltofd(client_con_obj.sockfd_tcp,1,down_size,client_data_times_pair);
		raise(SIGINT);

}


//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,char* s_hostaddr){

	char method_buff[PATHSIZE]={0};
	char req_buff[PATHSIZE/4]={0};

	sscanf(req_field,"%[^:]:%s",req_buff,method_buff);


	req_type the_type= str_to_req_type(req_buff);
	if(the_type==PLAY){
	if(!strs_are_strictly_equal(method_buff,"alsa")){

		fprintf(logstream,"Playing with ALSA library!\n");
		play_way=PLAY_ALSA;

	}
	else if(!strs_are_strictly_equal(method_buff,"pulse")){

		fprintf(logstream,"Playing with pulse_audio library!\n");
		play_way=PLAY_PA;

	}
	else{
		fprintf(logstream,"Unknown media library!\n");
		exit(-1);
	}
	}


	if(the_type==NA){
		printf(UNKNOWN_REQ);
		exit(-1);
	}
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);

	ip_cache_entry buff[PREV_ADDR_CACHE_MAX]={0};
	ip_cache_entry ent=(ip_cache_entry){{0},0};
	parse_ip_cache_entry(s_hostaddr,&ent);


	init_ip_addr_cache(&cache,buff);
	int cache_asked= !strs_are_strictly_equal(s_hostaddr,PREV_ADDR_STRING);
	int is_new=-1;
	if(!cache_asked)
	{
		is_new=find_ip_addr_entry(&ent,&cache);
		if(is_new<0){

			printf("Novo endereço inserido!!!!!\n");
		}
	}

	if(!cache_asked){
	client_con_obj.sockfd_tcp= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(client_con_obj.sockfd_tcp<0){

		raise(SIGINT);
	}
	init_addr(&server_ip_address,ent.hostname,ent.port);

	if(!tryConnect(&client_con_obj.sockfd_tcp,client_con_times_pair,&server_ip_address)){

		raise(SIGINT);
	}
	
	if(is_new<0){

		insert_ip_addr_entry(&ent,&cache);
		save_ip_addr_entry_cache(&cache);
	}
	}
	else{
		print_ip_cache(stdout,&cache);
		if(!try_cache_connect(&client_con_obj.sockfd_tcp,client_con_times_pair,&cache)){

			raise(SIGINT);
		}
	}
	print_sock_addr(client_con_obj.sockfd_tcp);
	init_con(&client_con_obj,client_con_obj.sockfd_tcp,CLIENT_C);
	
	getsockname(client_con_obj.sockfd_tcp,(struct sockaddr*)&client_con_obj.this_tcp_addr,socklenvar);
	greet(&client_con_obj,client_con_times_pair,client_con_obj.this_tcp_addr.sin_port);

	snprintf((char*)client_con_obj.tcp_data,DEF_DATASIZE-1,"%s %s",req_buff,file_name);

	con_send_tcp(&client_con_obj,client_data_times_pair);

	

	switch(the_type){

	case PLAY:
		play_func();
		break;
	case DOWN:
		down_func(file_name);
		break;
	case CONF:
		conf_func();
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

