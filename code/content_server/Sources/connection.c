#include "../../Includes/preprocessor.h"
#include <libgen.h>
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/openssl_stuff.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/engine.h"
#include "../Includes/connection.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/streamer_server.h"
#include "../Includes/upload_func.h"

static con_t server_con_obj;
static int sock_tcp;
static char file_name[PATHSIZE]={0},
	* dir_listing_str=NULL,
	file_path[PATHSIZE*3 +4]={0},
	req_string_buff[DEF_DATASIZE+1]={0},
	rep_file_path[PATHSIZE*3 +4]={0},
	rep_file_path_2[PATHSIZE*3 +4]={0};
static proto_arr proto_array={0};
static struct sigaction sa;

static req_type recvd_type=NA;
static struct stat file_info={0};

static int fp=-1;
static int fp_boundary=-1;

static void conStop(int useless){
	fp=(0*useless)+fp;

}

static void cleanup(void){
	close(fp);
	if(fp_boundary>=0){
		close(fp_boundary);
	}
	end_openssl_libs_server_side();
	close(sock_tcp);
	close_con(&server_con_obj,0,1);
	printf("Sent ports after minor server operation!\n");
	raise(SIGTERM);
}
static int open_file(char* filepath){

		int fp=-1;
		if((fp=open(filepath,O_RDONLY,0777))<0){
			setNonBlocking(&fp);
			printf("Accepted connection from %s, mas ficheiro %s e invalido. Conexao sera largada...\n",inet_ntoa(	server_con_obj.peer_tcp_addr.sin_addr),filepath);
	        }
		return fp;

}
static void send_download_sizes(int fd,char* file_path, struct stat file_info){
			clear_con_data(&server_con_obj);
			if(fd>0){
				stat(file_path,&file_info);
				snprintf((char*)server_con_obj.tcp_data,DEF_DATASIZE,"%ld %s %hhd",file_info.st_size,server_working_extension,is_wav_mode);
			}
			else{
				snprintf((char*)server_con_obj.tcp_data,DEF_DATASIZE,"-1");
			}
			int ret_send=con_send(&server_con_obj,server_data_times_pair);
			if(ret_send<0){
				if(server_con_obj.is_ssl){
					if(logging){
						fprintf(logstream,"Send to client failed!!!\n");
						ERR_print_errors_fp(logstream);
					}
				}
			}
}

//static get_filename_extension
void con_go(int sockfd_tcp){
			sa.sa_handler = conStop;
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = SA_RESTART;
			sigaction(SIGINT, &sa, NULL);
			sigaction(SIGPIPE, &sa, NULL);
			sigaction(SIGTERM, &sa, NULL);

			logging=cfg_server_logging;
			logstream=stdout;
			char extension_buff[EXTENSION_SIZE+1]={0};
			strncpy(extension_buff,server_working_extension,EXTENSION_SIZE+1);
			is_wav_mode=(int8_t)(!strs_are_strictly_equal(extension_buff,WAV_MODE_EXTENSION));
			if(is_wav_mode){
				printf("Launched in '.wav' mode!!!\n");
			}
			use_exit_func=1;
			exit_func_for_this_module=cleanup;
			sock_tcp=sockfd_tcp;
			unsigned char stream_cache_data[sizeof(mp3_stream_chunk)];
			InitializeSSL();
			init_openssl_libs_server_side();
			init_con(&server_con_obj,sock_tcp,SERVER_C,&port_mapper_ip_cache_entry,will_use_tls);
			if(greet(&server_con_obj,server_con_times_pair)){

				printf("Handshake failed!!!\n");
				cleanup();
				return;
			}
			con_read(&server_con_obj,server_data_times_pair);

			memcpy(req_string_buff,server_con_obj.tcp_data,DEF_DATASIZE);
			memcpy(proto_array,req_string_buff,PROTO_ARR_SIZE);
			recvd_type=ntohs(proto_array[0]);
			sscanf((char*)&req_string_buff[sizeof(uint16_t)],"%s",file_name);
			if(recvd_type==PLAY||recvd_type==DOWN){
				if(is_auto_mode){
					printf("Musica ignorada! Escolhendo a proxima da rotation!\n");
				}
				snprintf(file_path,sizeof(file_path)-1,"%s%s%s",curr_dir,
										is_auto_mode?server_auto_mode_rotation[curr_song_index_rotation]:file_name,
										server_working_extension);
			}
			else{
				switch(recvd_type){

					case PEEK:
						dir_listing_str=generateDirListing(file_name);
						snprintf(file_path,strnlen(dir_listing_str,PATHSIZE*2+1)+5,"%s",dir_listing_str);
						break;
					case CONFIG:
						produce_config_file();
						snprintf(file_path,sizeof(file_path)-1,"%s",TMP_CONFIG_FILE_PATH);
						break;
					case ROTATION:
						produce_rotation_file();
						snprintf(file_path,sizeof(file_path)-1,"%s",TMP_ROTATION_FILE_PATH);
						break;
					case REPORT:
						snprintf(file_path,sizeof(file_path)-1,"%s%s%s%s",curr_dir,file_name,server_working_extension,BOUNDARY_FILE_EXT);
						snprintf(rep_file_path,sizeof(rep_file_path)-1,"%s%s%s%s",curr_server_quarantine_dir_buff,file_name,server_working_extension,BOUNDARY_FILE_EXT);
						snprintf(rep_file_path_2,sizeof(rep_file_path_2),"%s",rep_file_path);
						break;
					default:
						printf(UNKNOWN_REQ);
						cleanup();
				}
			}
			clear_con_data(&server_con_obj);
			if((fp=open_file(file_path))<0){
				cleanup();
			}
			else{
				if(!is_wav_mode&&(recvd_type==PLAY)){
			 		printf("We are NOT in WAV mode bruuuhhhhh\n");
					char fp_boundary_path[PATHSIZE*10]={0};
					snprintf(fp_boundary_path,sizeof(fp_boundary_path)-1,"%s%s",file_path,BOUNDARY_FILE_EXT);
					if((fp_boundary=open_file(fp_boundary_path))<0){
						perror("Could not open boundary file!!!\n");
						cleanup();
					}
					printf("sucessfully opened boundary file at %s\n",fp_boundary_path);
				}
				else{

					printf("We are in wav mode! getting read of header at the start!\n");
				}
				printf("A file path é: %s\n",file_path);
				send_download_sizes(fp,file_path,file_info);
				switch(recvd_type){
				case PEEK:
					sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair,server_con_obj.is_ssl,server_con_obj.con_ssl);
					deleteDirListingFile();
					break;
				case DOWN:
					uploadtofd(server_con_obj.sockfd_tcp,fp,server_data_times_pair,server_con_obj.is_ssl,server_con_obj.con_ssl);
					break;
				case REPORT:
					_mkdir(dirname(rep_file_path_2));
					if(rename(file_path,rep_file_path)){
						fprintf(stderr,"rename: %s\nPath input: %s\nPath output: %s\n",strerror(errno),file_path,rep_file_path);
					}
					break;
				case PLAY:
					snprintf((char*)server_con_obj.tcp_data,DEF_DATASIZE,"%lu",server_chunk_size);
					if(con_send(&server_con_obj,server_data_times_pair)<=0){

						cleanup();
					}
					begin_stream(&server_con_obj,fp,fp_boundary,server_chunk_size,stream_cache_data);
					break;
				case CONFIG:
					sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair,server_con_obj.is_ssl,server_con_obj.con_ssl);
					remove(TMP_CONFIG_FILE_PATH);
					snprintf((char*)server_con_obj.tcp_data,DEF_DATASIZE,"\n\n\nServer contents successfully retrieved in full.\n\n");
					con_send(&server_con_obj,server_data_times_pair);
					break;
				case ROTATION:
					sendallfd(server_con_obj.sockfd_tcp,fp,server_data_times_pair,server_con_obj.is_ssl,server_con_obj.con_ssl);
					remove(TMP_ROTATION_FILE_PATH);
					snprintf((char*)server_con_obj.tcp_data,DEF_DATASIZE,"\n\n\nServer contents successfully retrieved in full.\n\n");
					con_send(&server_con_obj,server_data_times_pair);
					break;
				default:
					break;
				}
				cleanup();
			}
}

