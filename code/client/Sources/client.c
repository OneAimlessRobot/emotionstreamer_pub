#include "../Includes/preprocessor.h"
#include <libgen.h>
#include "../../mpg123-1.32.10/src/include/mpg123.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <ao/ao.h>
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/more_socket_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/openssl_stuff.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/ripped_code.h"
#include "../Includes/mp3module.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/chunk_player.h"
#include "../Includes/streamer_client.h"
#include "../Includes/client.h"
#include "../Includes/download_func.h"
#include "../Includes/ip_cache_file_ops.h"
#include "../Includes/terminal_mgmt.h"
#include "../../port_mapper/Includes/mapper.h"


struct stat file_info={0};
static atomic_int started=1;
static atomic_int is_on=1;
static int libao_initialized=0;
static int fp=-1;
static struct sigaction sa;
static char extension_from_server[PATHSIZE]={0};
static	char method_buff[PATHSIZE]={0},
	req_buff[DEF_DATASIZE/4]={0},
	file_path[PATHSIZE*3-1]={0},
	file_path2[PATHSIZE*3-1]={0};
static struct sockaddr_in server_ip_address,
	client_ip_address;
ip_cache_t cache=(ip_cache_t){NULL,0};
static con_t client_con_obj;
static method play_way=PLAY_PA;

static void clear_ports_and_quit(int signal,void* ptr){

	if(libao_initialized){

		ao_shutdown();
		libao_initialized=0;

	}
	send_port_back(htons(client_con_obj.this_tcp_addr.sin_port),&port_mapper_ip_cache_entry);
	free_attempted_ports(0,&port_mapper_ip_cache_entry);
	close_con(&client_con_obj,0,1);
	end_openssl_libs_client_side();
	fclose(logstream);
	close(fp);
	exit(signal+(0*((uint64_t)ptr)));
}
static void useless_handler(int useless){

	started=is_on=useless;
}
void exit_emergency_func(void){

	//clear_ports_and_quit(SIGINT,NULL);

}
static int64_t down_file_size(void){

		int64_t down_size=-1;
		clear_con_data(&client_con_obj);
		if(logging){
			printf("Recebendo tamanho!!!\n");
		}
		int ret_read=con_read(&client_con_obj,client_data_times_pair);
		sscanf((char*)client_con_obj.tcp_data,"%ld %s %hhd",&down_size,extension_from_server,&is_wav_mode);
		if(ret_read<=0){

			if(client_con_obj.is_ssl){
                                if(logging){
					fprintf(logstream,"Receive from server failed!!!\n");
					ERR_print_errors_fp(logstream);
                        	}
			}
			char* reason= down_size ? UNSUCESSFUL_DOWNLOAD_CON_ERROR:UNSUCESSFUL_DOWNLOAD_NOFILE ;
			if(logging){
				printf(UNSUCESSFUL_DOWNLOAD,reason);
				printf("String recebida do server upon entry: |%s|\n",(char*)client_con_obj.tcp_data);
			}
			clear_ports_and_quit(SIGINT,NULL);
		}
		clear_con_data(&client_con_obj);
		if(logging){
			printf(CONTENT_DOWNLOAD_INCOMMING,down_size,extension_from_server);
		}
		return down_size;

}
static void play_func(void){
		down_file_size();
		uint64_t chunk_size=0;
		con_read(&client_con_obj,client_data_times_pair);
		sscanf((char*)client_con_obj.tcp_data,"%lu",&chunk_size);
		player_init_stream(&client_con_obj,chunk_size,play_way);
}
static void down_func(char* file_name){

		int64_t down_size=down_file_size();
		snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,extension_from_server);
		snprintf(file_path2,sizeof(file_path2),"%s",file_path);
		_mkdir(dirname(file_path2));
		if((fp=creat(file_path,0777))<0){
                	if(logging){
				fprintf(stderr,"Nao foi possivel transferir ficheiro: %s!!!!\nPaths:\nPath1: %s\nPath2: %s\n",strerror(errno),file_path,file_path2);
			}
			clear_ports_and_quit(SIGINT,NULL);
                }
		if(stream_enable_ncurses){
			enable_ncurses();
		}
		downloadtofd(client_con_obj.sockfd_tcp,fp,down_size,client_data_times_pair,client_con_obj.is_ssl,client_con_obj.con_ssl);
		if(stream_enable_ncurses){
			endwin_wrapper();
		}
		if(logging){
			printf("A musica foi guardada em: %s\n",file_path);
		}
		clear_ports_and_quit(SIGINT,NULL);

}

static void peek_func(void){


		int down_size=down_file_size();
		if(logging){
			printf(CONTENT_PEEK_INCOMMING);
			printf("Conteúdos do server:\n");
		}
		readalltofd(client_con_obj.sockfd_tcp,1,down_size,client_data_times_pair,client_con_obj.is_ssl,client_con_obj.con_ssl);
		clear_ports_and_quit(SIGINT,NULL);

}
static void conf_func(void){

		int down_size=down_file_size();
		if(logging){
			printf(CONTENT_PEEK_INCOMMING);
		}
		readalltofd(client_con_obj.sockfd_tcp,1,down_size,client_data_times_pair,client_con_obj.is_ssl,client_con_obj.con_ssl);
		clear_ports_and_quit(SIGINT,NULL);
}


//Strings todas 0 ended
int clientStart(char* req_field,char* file_name){
	sa.sa_handler = useless_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);
	exit_func_for_this_module=exit_emergency_func;

	sscanf(req_field,"%[^:]:%s",req_buff,method_buff);
	req_type the_type= str_to_req_type(req_buff);
	if(the_type==NA){
		printf(UNKNOWN_REQ);
		fclose(logstream);
		exit(-1);
	}



	if(the_type==PLAY){
	if(!strs_are_strictly_equal(method_buff,"alsa")){

		fprintf(logstream,"Playing with ALSA library!\n");
		play_way=PLAY_ALSA;

	}
	else if(!strs_are_strictly_equal(method_buff,"pulse")){

		fprintf(logstream,"Playing with pulse_audio library!\n");
		play_way=PLAY_PA;

	}
	else if(!strs_are_strictly_equal(method_buff,"ao")){

		fprintf(logstream,"Playing with libao library!\n");
		ao_initialize();
    		libao_initialized=1;
		play_way=PLAY_AO;

	}
	else if(!strs_are_strictly_equal(method_buff,"oss")){

		fprintf(logstream,"Playing using bare file descriptors!\n(AKA rawest shit you've ever seen, my man)\n");
		play_way=PLAY_BARE;

	}
	else{
		fprintf(logstream,"Unknown media library!\nDefaulting to attempting Pulse!\n");
	}
	}


	ip_cache_entry buff[PREV_ADDR_CACHE_MAX]={0};


	if(init_ip_addr_cache(&cache,buff)){

	      clear_ports_and_quit(SIGINT,NULL);
	}
	int cache_asked= !strs_are_strictly_equal(server_ip_address_buff,PREV_ADDR_STRING);
	int is_new=-1;
	if(!cache_asked)
	{
		is_new=find_ip_addr_entry(&server_ip_cache_entry,&cache);
		if(is_new<0){

			printf("Novo endereço inserido!!!!!\n");
		}
	}

	if(!cache_asked){
	if(init_addr(&server_ip_address,server_ip_cache_entry.hostname,server_ip_cache_entry.port)){
		perror("Não conseguimos inicializar address de server no client!!!\n");
		clear_ports_and_quit(SIGINT,NULL);

	}
	init_openssl_libs_client_side();
	init_con(&client_con_obj,client_con_obj.sockfd_tcp,CLIENT_C,&port_mapper_ip_cache_entry,will_use_tls);
	connection_attempt_circuit(&client_con_obj.sockfd_tcp,clear_ports_and_quit,&client_ip_address,
                                &server_ip_address,
                                        &client_ip_cache_entry,&port_mapper_ip_cache_entry,client_con_times_pair,NULL);
	if(is_new<0){

		insert_ip_addr_entry(&server_ip_cache_entry,&cache);
		save_ip_addr_entry_cache(&cache);
	}
	}
	else{
		print_ip_cache(stdout,&cache);
		int result_con=0;
		if((result_con=try_cache_connect(&client_con_obj.sockfd_tcp,client_con_times_pair,&cache))<=0){
			clear_ports_and_quit(SIGINT,NULL);
        	}
	}
	if(logging){
		print_sock_addr(client_con_obj.sockfd_tcp);
	}
	setNonBlocking(&client_con_obj.sockfd_tcp);
	getsockname(client_con_obj.sockfd_tcp,(struct sockaddr*)&client_con_obj.this_tcp_addr,socklenvar);
	greet(&client_con_obj,client_con_times_pair);
	proto_arr proto_array={0};
	proto_array[0]=(uint16_t)htons(the_type);
	memcpy(client_con_obj.tcp_data,proto_array,sizeof(uint16_t));
	snprintf((char*)&client_con_obj.tcp_data[sizeof(uint16_t)],DEF_DATASIZE-2,"%s",file_name);
	con_send(&client_con_obj,client_data_times_pair);


	switch(the_type){

	case PLAY:
		play_func();
		break;
	case DOWN:
		down_func(file_name);
		break;
	case CONFIG:
		conf_func();
		break;
	case ROTATION:
		conf_func();
		break;
	case PEEK:
		peek_func();
		break;
	case REPORT:
		printf(REPORT_SENT_WITH_FILENAME,file_name);
		clear_ports_and_quit(SIGINT,NULL);
        	break;
	default:
		break;
	}
	return 0;
}

