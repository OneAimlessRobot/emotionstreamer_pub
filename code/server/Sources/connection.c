#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/connection.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/streamer_server.h"



static con_t server_con_obj;

void sigint_handler(int useless){


	close_con(&server_con_obj + (0*useless));

}
void sigpipe_handler(int useless){

	sigint_handler(useless);

}


void con_go(int sockfd_tcp,int curr_port){
				signal(SIGINT, sigint_handler);
				signal(SIGPIPE, sigpipe_handler);

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
				char* dir_listing_str=NULL;
				int fp;

				init_con(&server_con_obj,sockfd_tcp,SERVER_C,con_buffs);

				greet(&server_con_obj,curr_port);
				
				clear_con_data(&server_con_obj);

				con_read_tcp(&server_con_obj,server_data_times_pair);
				
				sscanf((char*)server_con_obj.tcp_data,"%s %s",req_buff,file_name);
				
				printf("Buff recebido: %s\n",server_con_obj.tcp_data);
				
				req_type recvd_type= str_to_req_type(req_buff);
				//(Quis ler o request e o filename em transferencias diferentes)


				switch(recvd_type){

					case PLAY:
						snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,EXTENSION);
						break;
					case PEEK:
						dir_listing_str=generateDirListing();
						snprintf(file_path,strlen(dir_listing_str)+1,"%s",dir_listing_str);
						break;
					default:
						printf(UNKNOWN_REQ);
						raise(SIGINT);
						return;
				}
				if((fp=open(file_path,O_RDONLY,0777))<0){
					
					//setNonBlocking(fp);
					printf("Accepted connection from %s, mas ficheiro %s e invalido. Conexao sera largada...\n",inet_ntoa(server_con_obj.peer_tcp_addr.sin_addr),file_path);
	                       		perror("Nao foi possivel abrir nada!!!!\n");
	                        	if(dir_listing_str){
						free(dir_listing_str);
						dir_listing_str=NULL;
					}
					raise(SIGINT);
	                	}
				else{
				
					
					if(recvd_type==PEEK){
						sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair);
						deleteDirListingFile();
						raise(SIGINT);
					}
					else if(recvd_type==PLAY){

						printf("A file path é: %s\n",file_path);
						read(fp,file_name,44);//GET RID OF WAV HEADER! IMPORTANT! 44 BYTES!
						begin_stream(&server_con_obj,fp,stream_cache_data);
					}

				}
}

