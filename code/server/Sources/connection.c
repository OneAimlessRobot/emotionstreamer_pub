#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/engine.h"
#include "../Includes/connection.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/streamer_server.h"
#include "../Includes/upload_func.h"

static con_t server_con_obj;
static int sock_tcp;
static uint16_t port;
int fp=-1;
static void cleanup(int useless){
	close(fp + (0*useless));
	close(sock_tcp);
	close_con(&server_con_obj);
	raise(SIGTERM);

}
static int open_file(char* filepath){

		int fp=-1;
		if((fp=open(filepath,O_RDONLY,0777))<0){
			setNonBlocking(fp);
			printf("Accepted connection from %s, mas ficheiro %s e invalido. Conexao sera largada...\n",inet_ntoa(server_con_obj.peer_tcp_addr.sin_addr),filepath);
                       	
	        }
		return fp;

}
static void send_download_sizes(int fd,char* file_path, struct stat file_info){
			clear_con_data(&server_con_obj);
			if(fd>0){
				stat(file_path,&file_info);
				snprintf((char*)server_con_obj.udp_data,DEF_DATASIZE,"%ld",file_info.st_size);
				con_send_udp(&server_con_obj,server_data_times_pair);
				clear_con_data(&server_con_obj);
				con_read_udp_ack(&server_con_obj,server_data_times_pair);
				if(strs_are_strictly_equal((char*)server_con_obj.ack_udp_data,CON_STRING)){

					printf("Ma resposta do cliente!!!!\n Abortando conexao!\nResposta: |%s|\n",(char*)server_con_obj.ack_udp_data);
					raise(SIGINT);
				}
			}
			else{
				snprintf((char*)server_con_obj.udp_data,DEF_DATASIZE,"-1");
				con_send_udp(&server_con_obj,server_data_times_pair);
			}
}

void con_go(int sockfd_tcp, uint16_t curr_port){

		signal(SIGINT,cleanup);
			
			sock_tcp=sockfd_tcp;
			port=curr_port;
				unsigned char stream_cache_data[server_chunk_size];

				char file_name[PATHSIZE]={0};
				char file_path[PATHSIZE*2 +4]={0};
				
				char req_buff[PATHSIZE]={0};
				struct stat file_info={0};

				init_con(&server_con_obj,sock_tcp,SERVER_C);

				greet(&server_con_obj,server_con_times_pair,port);
				
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
						char* dir_listing_str=generateDirListing(file_name);
						snprintf(file_path,strnlen(dir_listing_str,PATHSIZE*2+1)+5,"%s",dir_listing_str);
						break;
					case CONF:
						printf("Conf pedido!\n");
						produce_config_file();
						snprintf(file_path,sizeof(file_path)-1,"%s",TMP_CONFIG_FILE_PATH);
						break;
					default:
						printf(UNKNOWN_REQ);
						raise(SIGINT);
				}
				clear_con_data(&server_con_obj);
				if((fp=open_file(file_path))<0){
					raise(SIGINT);
				}
				else{
					printf("A file path é: %s\n",file_path);
					send_download_sizes(fp,file_path,file_info);
					switch(recvd_type){
					case PEEK:
						sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair);
						deleteDirListingFile();
						break;
					case DOWN:
						uploadtofd(server_con_obj.sockfd_tcp,fp,server_data_times_pair);
						break;
					case PLAY:
						read(fp,file_name,44);//GET RID OF WAV HEADER! IMPORTANT! 44 BYTES!
						snprintf((char*)server_con_obj.udp_data,DEF_DATASIZE,"%hu",server_chunk_size);
						if(con_send_udp(&server_con_obj,server_data_times_pair)<=0){

							raise(SIGINT);
						}
						begin_stream(&server_con_obj,fp,server_chunk_size,stream_cache_data);
						break;
					case CONF:
						sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair);
						remove(TMP_CONFIG_FILE_PATH);
						break;
					default:
						break;
					}
					raise(SIGINT);
				}
}
