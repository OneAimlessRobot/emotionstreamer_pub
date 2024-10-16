#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../Includes/load_html.h"
#include "../Includes/connection.h"
#include "../Includes/engine.h"

static connection con_obj;
static int result=0;
//retorna 0 se o objecto tava vazio. 1 se n tava


//null terminated string
static void handle_error(char* prompt){

	if(result){
		perror(prompt);
		raise(SIGINT);
	}

}
static void sigint_handler(int useless){
	
	printf("Recebemos sinal dentro de um processo de conexão. Sinal: %d\n",useless);
	con_obj.is_on=0;
	close(con_obj.sockfd_tcp);
	exit(useless);
}
static void init_con(int sockfd,struct sockaddr_in * addr){

				con_obj.is_on=1;
				con_obj.sockfd_tcp=sockfd;
				memcpy(&(con_obj.tcp_from_addr),addr,*socklenvar);
				memset(con_obj.file_path,0,PATHSIZE);
				print_addr_aux("Addresss do cliente:",&con_obj.tcp_from_addr);
				
}


void con_go(int sockfd,struct sockaddr_in* c_addr){
				
				signal(SIGINT, sigint_handler);
				signal(SIGPIPE, sigint_handler);
				
				
				//struct sockaddr_in curr_con_addr;
				char in_file_name[PATHSIZE]={0},
				req_type_in_buff[BUFFSIZE]={0},
				file_path[PATHSIZE]={0};
				char* dir_listing_str=NULL;
				int fp;
				struct stat file_info;
				
				//buff[DEF_DATASIZE]={0},
				init_con(sockfd,c_addr);

				//Lendo o request do cliente
				result=(readsome(con_obj.sockfd_tcp,req_type_in_buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR)<=0);
				printf("Request recebido: %s\n",req_type_in_buff);
				handle_error("Read error");
				req_type recvd_type= str_to_req_type(req_type_in_buff);
				
				//Se depois deste send...
				result=(sendsome(con_obj.sockfd_tcp,req_type_in_buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR)<=0);
				handle_error("Send error");
				//este read suceder, n importa o que recebe, o cliente recebeu autorizaçao para nos dar o filename
				
				result=(readsome(con_obj.sockfd_tcp,in_file_name,PATHSIZE-1,CLIENT_DATA_TIMES_PAIR)<=0);
				printf("Filename recebido: %s\n",in_file_name);
				handle_error("Recv error");
				//(Quis ler o request e o filename em transferencias diferentes)
				switch(recvd_type){

					case PLAY:
						snprintf(file_path,3*PATHSIZE-1,"%s%s",curr_dir,in_file_name);
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
				memcpy(con_obj.file_path,file_path,PATHSIZE);
				printf("A file path é: %s\n",file_path);
				if((fp=open(file_path,O_RDONLY,0777))<0){
					
	                        	long flags2= fcntl(fp,F_GETFL);
	                        	flags2 |= O_NONBLOCK;
	                        	fcntl(fp,F_SETFL,flags2);

	                        	printf("Accepted connection from %s, mas ficheiro %s e invalido. Conexao sera largada...\n",inet_ntoa(con_obj.tcp_from_addr.sin_addr),file_path);
	                       		perror("Nao foi possivel abrir nada!!!!\n");
	                        	if(dir_listing_str){
						free(dir_listing_str);
						dir_listing_str=NULL;
					}
					raise(SIGINT);
	                	}
				else{
				
					char file_size_buff_out[DEF_DATASIZE+1]={0};
					//char file_size_buff_back[DEF_DATASIZE+1]={0};;
					fstat(fp,&file_info);
					snprintf(file_size_buff_out,DEF_DATASIZE,"%ld",file_info.st_size);
					//enviando datasize
					sendsome(con_obj.sockfd_tcp,file_size_buff_out,DEF_DATASIZE,SERVER_DATA_TIMES_PAIR);
					handle_error("Send error");
					//Se este read suceder, n importa o que recebe, significa que o cliente recebeu o size
					readsome(con_obj.sockfd_tcp,file_size_buff_out,DEF_DATASIZE,SERVER_DATA_TIMES_PAIR);
					handle_error("Read error");

					sendallfd(con_obj.sockfd_tcp,fp,SERVER_DATA_TIMES_PAIR);
					
					if(recvd_type==PEEK){
						deleteDirListingFile();
					}
					close(fp);
				}
				raise(SIGINT);
}

