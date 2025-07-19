#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
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
#include "../Includes/upload_func.h"

static con_t server_con_obj;
static int sock_tcp;
int fp=-1;

static void cleanup(int useless){
	close(fp + (0*useless));
	close(sock_tcp);
	send_ports_back(&server_con_obj);
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
				snprintf((char*)server_con_obj.udp_data,DEF_DATASIZE,"%ld %hd %s %hd",file_info.st_size,server_transmission_protocol,server_working_extension,is_wav_mode);
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
//static get_filename_extension
void con_go(int sockfd_tcp, uint16_t curr_port){

		signal(SIGINT,cleanup);
			
			sock_tcp=sockfd_tcp;
				unsigned char stream_cache_data[server_chunk_size];

				char file_name[PATHSIZE]={0};
				char file_path[PATHSIZE*3 +4]={0};
				
				char req_buff[PATHSIZE]={0};
				struct stat file_info={0};
				char hp_udp_ack_buff[2*DEF_DATASIZE]={0};
				char hp_udp_buff[2*DEF_DATASIZE]={0};
				init_con(&server_con_obj,sock_tcp,SERVER_C,curr_port,&server_port_mapper_ip_cache_entry);

				greet(&server_con_obj,server_con_times_pair,server_holepunching_times_pair);
				
				clear_con_data(&server_con_obj);

				con_read_udp(&server_con_obj,server_data_times_pair);
				
				sscanf((char*)server_con_obj.udp_data,"%s %s",req_buff,file_name);
				
				printf("Buff recebido:\n\"%s\"\n",server_con_obj.udp_data);
				
				clear_con_data(&server_con_obj);


			        snprintf((char*)hp_udp_buff,2*DEF_DATASIZE-1,"Buff recebido:\n\"%s\"\n",(char*)server_con_obj.udp_data);

				

				con_send_udp(&server_con_obj,server_data_times_pair);

				clear_con_data(&server_con_obj);

				con_read_udp_ack(&server_con_obj,server_data_times_pair);

				snprintf((char*)hp_udp_ack_buff,2*DEF_DATASIZE-1,"Buff recebido (TEST UDP ACK):\n\"%s\"\n",(char*)server_con_obj.ack_udp_data);
				printf("Result from TEST UDP ACK:\n\"%s\"\n",hp_udp_ack_buff);
			        

				con_send_udp_ack(&server_con_obj,server_data_times_pair);

				req_type recvd_type= str_to_req_type(req_buff);
				//(Quis ler o request e o filename em transferencias diferentes)


				switch(recvd_type){

					case PLAY:
						
						printf("Play pedido!\n");
						snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,server_working_extension);
						break;
					case DOWN:
						printf("Download pedido!\n");
						snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,file_name,server_working_extension);
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
						printf(UNKNOWN_REQ,req_buff);
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
						printf("We are in wav mode! getting read of header at the start!\n");
						snprintf((char*)server_con_obj.udp_data,DEF_DATASIZE,"%lu",server_chunk_size);
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

