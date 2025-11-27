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
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/more_socket_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
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

struct stat file_info={0};
static atomic_int started=0;
static atomic_int is_on=0;
static int forceful_teardown=0;
static int fp=-1;
static struct sigaction sa;
static uint16_t port=0;
static char extension_from_server[PATHSIZE]={0};
static	char method_buff[PATHSIZE]={0},
	req_buff[PATHSIZE/4]={0},
	file_path[PATHSIZE*3-1]={0},
	file_path2[PATHSIZE*3-1]={0};
static struct sockaddr_in server_ip_address,
	client_ip_address;
ip_cache_t cache=(ip_cache_t){NULL,0};
static con_t client_con_obj;
static method play_way=PLAY_PA;
static void clear_ports_and_quit(int signal){

	send_port_back(port,&client_port_mapper_ip_cache_entry);
	close_con(&client_con_obj,0);
	fclose(logstream);
	close(fp);
	exit(signal);
}
static void useless_handler(int useless){

	started=is_on=useless;
}


static int64_t down_file_size(void){

		int64_t down_size=-1;
		clear_con_data(&client_con_obj);
		printf("Recebendo tamanho!!!\n");
		con_read_tcp(&client_con_obj,client_data_times_pair);
		sscanf((char*)client_con_obj.tcp_data,"%ld %s %hhd",&down_size,extension_from_server,&is_wav_mode);
		if(down_size<=0){

			char* reason= down_size ? UNSUCESSFUL_DOWNLOAD_NOFILE :UNSUCESSFUL_DOWNLOAD_CON_ERROR;
			printf(UNSUCESSFUL_DOWNLOAD,reason);
			clear_ports_and_quit(SIGINT);
		}
		clear_con_data(&client_con_obj);
		printf(CONTENT_DOWNLOAD_INCOMMING,down_size,extension_from_server);
		return down_size;

}
static void play_func(void){
		down_file_size();
		uint64_t chunk_size=0;
		con_read_tcp(&client_con_obj,client_data_times_pair);
		sscanf((char*)client_con_obj.tcp_data,"%luï",&chunk_size);
		greet(&client_con_obj,client_con_times_pair);
		player_init_stream(&client_con_obj,chunk_size,play_way);
}
static void down_func(char* file_name){

		int64_t down_size=down_file_size();
		snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,extension_from_server);
		snprintf(file_path2,sizeof(file_path2),"%s",file_path);
		_mkdir(dirname(file_path2));
		if((fp=creat(file_path,0777))<0){
                		fprintf(stderr,"Nao foi possivel transferir ficheiro: %s!!!!\nPaths:\nPath1: %s\nPath2: %s\n",strerror(errno),file_path,file_path2);
				clear_ports_and_quit(SIGINT);
                }
		if(stream_enable_ncurses){
			enable_ncurses();
		}
		downloadtofd(client_con_obj.sockfd_tcp,fp,down_size,client_data_times_pair);
		if(stream_enable_ncurses){
			endwin_wrapper();
		}
		printf("A musica foi guardada em: %s\n",file_path);
		clear_ports_and_quit(SIGINT);

}
static void peek_func(void){


		int down_size=down_file_size();
		printf(CONTENT_PEEK_INCOMMING);
		readalltofd(client_con_obj.sockfd_tcp,1,down_size,client_data_times_pair);
		clear_ports_and_quit(SIGINT);

}
static void conf_func(void){

		int down_size=down_file_size();
		printf(CONTENT_PEEK_INCOMMING);
		readalltofd(client_con_obj.sockfd_tcp,1,down_size,client_data_times_pair);
		clear_ports_and_quit(SIGINT);
}


//Strings todas 0 ended
int clientStart(char* req_field,char* file_name){
	sa.sa_handler = useless_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);


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
	else if(!strs_are_strictly_equal(method_buff,"ao")){

		fprintf(logstream,"Playing with libao library!\n");
		ao_initialize();
    		play_way=PLAY_AO;

	}
	else if(!strs_are_strictly_equal(method_buff,"oss")){

		fprintf(logstream,"Playing using bare file descriptors!\n(AKA rawest shit you've ever seen, my man)\n");
		ao_initialize();
    		play_way=PLAY_BARE;

	}
	else{
		fprintf(logstream,"Unknown media library!\n");
		fclose(logstream);
		exit(-1);
	}
	}


	if(the_type==NA){
		printf(UNKNOWN_REQ,req_buff);
		fclose(logstream);
		exit(-1);
	}
	ip_cache_entry buff[PREV_ADDR_CACHE_MAX]={0};


	if(init_ip_addr_cache(&cache,buff)){

	      clear_ports_and_quit(SIGINT);
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
	client_con_obj.sockfd_tcp= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(client_con_obj.sockfd_tcp<0){

		clear_ports_and_quit(SIGINT);
        }
    	set_sock_reuseaddr(&client_con_obj.sockfd_tcp,1);
	setNonBlocking(&client_con_obj.sockfd_tcp);
	if(init_addr(&server_ip_address,server_ip_cache_entry.hostname,server_ip_cache_entry.port)){
		perror("Não conseguimos inicializar address de server no client!!!\n");
		clear_ports_and_quit(SIGINT);

	}

	ask_for_port(&port,&client_port_mapper_ip_cache_entry);
	if(!port||init_addr(&client_ip_address,client_ip_cache_entry.hostname,port)){
		perror("Não conseguimos inicializar address no client!!!\n");
		clear_ports_and_quit(SIGINT);

	}

	if(bind(client_con_obj.sockfd_tcp,(struct sockaddr *)&client_ip_address,socklenvar[1])){
		perror("Não conseguimos dar bind na socket do client!!!\n");
		print_addr_aux("Este é o address:",&client_ip_address);
		clear_ports_and_quit(SIGINT);
	}
	else{

		print_addr_aux("Bind com sucesso!!!:",&client_ip_address);
		setLinger(&client_con_obj.sockfd_tcp,1,1);
	}

	int result_con=0;
	if(!(result_con=tryConnect(&client_con_obj.sockfd_tcp,client_con_times_pair,&server_ip_address))){
		clear_ports_and_quit(SIGINT);
        }
	else if(result_con<0){
		if(logging){

			fprintf(logstream,"Initiating forceful teardown!\nResult = %d\n",result_con);
		}
		forceful_teardown=1;
		clear_ports_and_quit(SIGINT);
        }

	if(is_new<0){

		insert_ip_addr_entry(&server_ip_cache_entry,&cache);
		save_ip_addr_entry_cache(&cache);
	}
	}
	else{
		print_ip_cache(stdout,&cache);
		int result_con=0;
		if((result_con=try_cache_connect(&client_con_obj.sockfd_tcp,client_con_times_pair,&cache))<=0){
			forceful_teardown=(result_con!=0);
			clear_ports_and_quit(SIGINT);
        	}
	}
	print_sock_addr(client_con_obj.sockfd_tcp);
	setNonBlocking(&client_con_obj.sockfd_tcp);
	init_con(&client_con_obj,client_con_obj.sockfd_tcp,CLIENT_C,client_con_obj.this_tcp_addr.sin_port,&client_port_mapper_ip_cache_entry);
	getsockname(client_con_obj.sockfd_tcp,(struct sockaddr*)&client_con_obj.this_tcp_addr,socklenvar);
	snprintf((char*)client_con_obj.tcp_data,2*DEF_DATASIZE-1,"%s %s",req_buff,file_name);

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
	case REPORT:
		printf(REPORT_SENT_WITH_FILENAME,file_name);
		clear_ports_and_quit(SIGINT);
        	break;
	default:
		printf(UNKNOWN_REQ,req_buff);
		clear_ports_and_quit(SIGINT);
        	break;
	}
	return 0;
}

