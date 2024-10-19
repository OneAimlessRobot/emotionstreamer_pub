#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/connection.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/streamer_server.h"

static con_t server_con_obj;



void con_go(int sockfd_tcp){

				char file_name[DEF_DATASIZE]={0};
				char file_path[DEF_DATASIZE*2 +2]={0};
				char req_buff[PATHSIZE]={0};
				char* dir_listing_str=NULL;
				int fp;

				init_con(&server_con_obj,sockfd_tcp,SERVER_C);

				greet(&server_con_obj);
				
				clear_con_data(&server_con_obj);

				con_read_tcp(&server_con_obj,SERVER_DATA_TIMES_PAIR);
				
				sscanf((char*)server_con_obj.data,"%s %s",req_buff,file_name);
				
				printf("Buff recebido: %s\n",server_con_obj.data);
				
				req_type recvd_type= str_to_req_type(req_buff);
				//(Quis ler o request e o filename em transferencias diferentes)


				switch(recvd_type){

					case PLAY:
						snprintf(file_path,sizeof(file_path)-1,"%s%s",curr_dir,file_name);
						break;
					case PEEK:
						dir_listing_str=generateDirListing();
						snprintf(file_path,strlen(dir_listing_str)+1,"%s",dir_listing_str);
						break;
					default:
						printf(UNKNOWN_REQ);
						raise(SIGINT);
						break;
				}
				printf("A file path é: %s\n",file_path);
				
				if((fp=open(file_path,O_RDONLY,0777))<0){
					
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
						sendallfd(server_con_obj.sockfd_tcp,fp,SERVER_DATA_TIMES_PAIR);
						deleteDirListingFile();
					}
					else if(recvd_type==PLAY){
					
						
						begin_stream(&server_con_obj,fp);
					
						//printf("Not implemented\n");
					}

				}
				raise(SIGINT);
}

