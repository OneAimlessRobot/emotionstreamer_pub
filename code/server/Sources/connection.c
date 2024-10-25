#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/configs.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/engine.h"
#include "../Includes/connection.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/streamer_server.h"



static con_t server_con_obj;
int server_tcp_sock=-1;

void sigint_handler(int useless){


	close_con(&server_con_obj + (0*useless));
	close(server_tcp_sock);
	exit(useless);
}
void sigpipe_handler(int useless){

	sigint_handler(useless);

}
static void send_download_sizes(int fd,char* file_path, struct stat file_info){
			clear_con_data(&server_con_obj);
			if(fd>0){
				stat(file_path,&file_info);
				snprintf((char*)server_con_obj.udp_data,cfg_datasize,"%ld",file_info.st_size);
				con_send_udp(&server_con_obj,server_data_times_pair);
				clear_con_data(&server_con_obj);
				con_read_udp_ack(&server_con_obj,server_data_times_pair);
				if(strs_are_strictly_equal((char*)server_con_obj.ack_udp_data,CON_STRING)){

					printf("Ma resposta do cliente!!!!\n Abortando conexao!\n");
					raise(SIGINT);
				}
			}
			else{
				snprintf((char*)server_con_obj.tcp_data,cfg_datasize,"-1");
				con_send_udp(&server_con_obj,server_data_times_pair);
				
			}
}

void con_go(server_state* state,int sockfd_tcp,int curr_port){
				signal(SIGINT, sigint_handler);
				signal(SIGPIPE, sigpipe_handler);
				server_tcp_sock=state->server_sock_tcp;
				unsigned char tcp_data[cfg_datasize+1];
				unsigned char udp_data[cfg_datasize+1];
				unsigned char ack_udp_data[cfg_datasize+1];
				unsigned char stream_cache_data[cfg_chunk_size];


				buff_triple con_buffs={NULL};
				con_buffs[0]=tcp_data;
				con_buffs[1]=udp_data;
				con_buffs[2]=ack_udp_data;
				char file_name[cfg_datasize];
				memset(file_name,0,cfg_datasize);
				char file_path[cfg_datasize*2 +2];
				memset(file_path,0,cfg_datasize+2+2);
				char req_buff[PATHSIZE]={0};
				int fp;
				struct stat file_info={0};

				init_con(&server_con_obj,sockfd_tcp,SERVER_C,con_buffs);

				greet(&server_con_obj,server_con_times_pair,curr_port);
				
				clear_con_data(&server_con_obj);

				con_read_tcp(&server_con_obj,server_data_times_pair);
				
				sscanf((char*)server_con_obj.tcp_data,"%s %s",req_buff,file_name);
				
				printf("Buff recebido: %s\n",server_con_obj.tcp_data);
				
				req_type recvd_type= str_to_req_type(req_buff);
				//(Quis ler o request e o filename em transferencias diferentes)


				switch(recvd_type){

					case PLAY:
						
						printf("Play pedido!\n");
						snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,EXTENSION);
						break;
					case DOWN:
						printf("Download pedido!\n");
						snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,EXTENSION);
						break;
					case PEEK:
						printf("Peek pedido!\n");
						char* dir_listing_str=generateDirListing();
						snprintf(file_path,strlen(dir_listing_str)+1,"%s",dir_listing_str);
						break;
					default:
						printf(UNKNOWN_REQ);
						raise(SIGINT);
						return;
				}
				clear_con_data(&server_con_obj);
				if((fp=open(file_path,O_RDONLY,0777))<0){
					setNonBlocking(fp);
					printf("Accepted connection from %s, mas ficheiro %s e invalido. Conexao sera largada...\n",inet_ntoa(server_con_obj.peer_tcp_addr.sin_addr),file_path);
	                       		perror("Nao foi possivel abrir nada!!!!\n");
	                        	raise(SIGINT);
	                	}
				else{
					send_download_sizes(fp,file_path,file_info);
					if(recvd_type==PEEK){
						sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair);
						deleteDirListingFile();
						raise(SIGINT);
					}
					if(recvd_type==DOWN){
						printf("A file path é: %s\n",file_path);
						sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair);
						raise(SIGINT);
					}
					if(recvd_type==PLAY){
						printf("A file path é: %s\n",file_path);
						read(fp,file_name,44);//GET RID OF WAV HEADER! IMPORTANT! 44 BYTES!
						begin_stream(&server_con_obj,fp,stream_cache_data);
						raise(SIGINT);
					}
				}
}

