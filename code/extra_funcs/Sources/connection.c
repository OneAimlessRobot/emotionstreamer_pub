#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"
#include "../Includes/openssl_stuff.h"
#include "../Includes/sock_ops.h"
#include "../Includes/more_socket_ops.h"
#include "../Includes/sockio_udp.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/ip_cache_file.h"
#include "../Includes/connection.h"
#include "../../port_mapper/Includes/mapper.h"
#include "../Includes/generalized_config.h"


void clear_con_data(con_t* con_obj){

	memset(con_obj->tcp_data,0,DEF_DATASIZE+1);


}

static void prep_con(con_t* con_obj){

	memset(con_obj,0,sizeof(con_t));

}


void print_port_arr(void){

	char printed_buff[DEF_DATASIZE]={0};
	char* ptr=printed_buff;
	ptr+=snprintf(ptr,sizeof(printed_buff)-(ptr-printed_buff),"this is the state of the port array currently! There are %hu ports!\n\n",attempted_port_arr[0]);
	for(uint16_t i=0;i<attempted_port_arr[0];i++){
		ptr+=snprintf(ptr,sizeof(printed_buff)-(ptr-printed_buff),"port number %hu: %hu\n",i+1,attempted_port_arr[i+1]);
	}
	ptr+=snprintf(ptr,sizeof(printed_buff)-(ptr-printed_buff),"\nEnd of port array state\n\n");
	printf("%s\n",printed_buff);

}

void send_port_back(uint16_t port,ip_cache_entry* ent){
	if(!port){
		if(logging){
			fprintf(logstream,"Refusing to send back null port at send_port_back!\n");
		}
		return;
	}
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(tmp_socket<0){
		if(logging){
			perror("Criação de socket para conectar ao port mapper para devolver porta unica mal sucedida. Abortando\n");
		}
		raise(SIGINT);
		return;
	}

	if(init_addr(&addr, ent->hostname,ent->port)){

		if(logging){
			perror("Iniciacao de address para conectar ao port mapper para devolver porta unica mal sucedida. Abortando\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	int result_con=0;
	if((result_con=tryConnect(&tmp_socket,port_mapper_times_pair,&addr))<=0){
		if(logging){
			perror("Conexão ao port mapper para devolver porta unica mal sucedida! Abortando\n");
		}
		socket_close(&tmp_socket,result_con!=0);
		raise(SIGINT);
		return;
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_AWKWARD_LEAVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("O port mapper nâo recebeu o nosso request!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("Não conseguimos enviar o request de fecho de porta ao port mapper!!!!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	result=sendsome(tmp_socket,(char*)&port,sizeof(port),port_mapper_times_pair);
	if(result<=0){
		if(logging){

			perror("Não conseguimos enviar a porta para fechar portas ao port mapper!!!!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	else{
		if(logging){
			fprintf(logstream,"Portas enviadas e coiso!!!!!!\nA porta enviada foi %d\n",port);
		}
		close(tmp_socket);
	}

}
void send_ports_back(ip_cache_entry* ent,uint16_t port_that_works){
	if(attempted_port_arr[0]<=0){
		if(logging){
			fprintf(logstream,"Attempt was made to send back zero ports at send_ports_back!\nIgnoring...\n");
		}
		return;
	}
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(tmp_socket<0){
		if(logging){
			perror("Criação de socket para conectar ao port mapper para devolver portas mal sucedida. Abortando\n");
		}
		raise(SIGINT);
		return;
	}

	if(init_addr(&addr, ent->hostname,ent->port)){

		if(logging){
			perror("Iniciacao de address para conectar ao port mapper para devolver portas mal sucedida. Abortando\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	int result_con=0;
	if((result_con=tryConnect(&tmp_socket,port_mapper_times_pair,&addr))<=0){
		if(logging){
			perror("Conexão ao port mapper para devolver portas mal sucedida! Abortando\n");
		}
		socket_close(&tmp_socket,result_con!=0);
		raise(SIGINT);
		return;
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_LEAVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("O port mapper nâo recebeu o nosso request!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("Não conseguimos enviar o request de fecho de portas ao port mapper!!!!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	if(logging){
		fprintf(logstream,"Init port array state!\n");
		print_port_arr();
	}
	attempted_port_arr[0]-=(port_that_works!=0);
	if(port_that_works){
		if(logging){
			fprintf(logstream,"port array state\nWe are about to move anything beyond slot %hu back 1 position towards slot %hu\nWe will move %hu items of size %lu\nThere are %hu ports to free right now\n",port_that_works+1,port_that_works,attempted_port_arr[0]-(port_that_works)+1,sizeof(attempted_port_arr[0]),attempted_port_arr[0]);
			print_port_arr();
		}
		memmove(&attempted_port_arr[port_that_works],&attempted_port_arr[port_that_works+1],(attempted_port_arr[0]-(port_that_works)+1)*sizeof(attempted_port_arr[0]));
	}
	result=sendsome(tmp_socket,(char*)attempted_port_arr,sizeof(port_array),port_mapper_times_pair);
	memset(attempted_port_arr,0,sizeof(port_array));
	if(result<=0){
		if(logging){
			perror("Não conseguimos enviar as portas para fechar portas ao port mapper!!!!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	else{
		if(logging){
			fprintf(logstream,"End port array state\nWe sent all the ports!!!\n");
			print_port_arr();
		}
		close(tmp_socket);
	}

}

void close_con(con_t* con_obj,int RIGHT_NOW,int close_for_good){
	if(con_obj->is_on){
		if(con_obj->sockfd_tcp>=0){
			if(logging){
				fprintf(logstream,"Fechamos socket numero %d!!!!\n",con_obj->sockfd_tcp);
			}
			socket_close(&(con_obj->sockfd_tcp),RIGHT_NOW);
		}
		con_obj->sockfd_tcp=-1;
		con_obj->is_on=(0||(close_for_good!=0));
		if(logging){
			fprintf(logstream,"Fechamos conexão!!!!\nDe vez? %s\n",close_for_good?"Yes!":"No...");
		}
		if(con_obj->con_ssl){
			if(logging){
				fprintf(logstream,"Closing ssl on connection named %s!\n",con_obj->con_name);
			}
			ShutdownSSL(&con_obj->con_ssl);
		}
	}
	else{

		if(logging){
			fprintf(logstream,"Conexão nao aberta. portanto, fechar não será tentado\n");
		}
	}
}

void give_name_to_con(con_t* con_obj, const char* name){

	if(name){
		memcpy(con_obj->con_name,name,min(strlen(name),CON_NAME_MAX_LENGTH));
	}

}
void init_con(con_t* con_obj,int sockfd_tcp,con_type type,ip_cache_entry *ent,uint8_t is_ssl){

				prep_con(con_obj);
				give_name_to_con(con_obj,CON_NAME_DEFAULT);
				con_obj->is_on=1;
				con_obj->type=type;
                                con_obj->sockfd_tcp=sockfd_tcp;

				socklen_t socklen_in=sizeof(struct sockaddr_in);
				//socklen_t socklen=sizeof(struct sockaddr);

				getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_tcp_addr),&socklen_in);

				socklen_in=sizeof(struct sockaddr_in);
			        //socklen=sizeof(struct sockaddr);

                                getsockname(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->this_tcp_addr),&socklen_in);

				memcpy(&con_obj->port_mapper_entry,ent,sizeof(ip_cache_entry));
				init_addr(&con_obj->port_mapper_addr, con_obj->port_mapper_entry.hostname,con_obj->port_mapper_entry.port);

				memset(con_obj->tcp_data,0,DEF_DATASIZE+1);
				con_obj->is_ssl=is_ssl;

}

int con_send_tcp(con_t* con_obj,int_pair pair){

	return con_obj->is_ssl?sendsome_ssl(con_obj->con_ssl,(const char*)con_obj->tcp_data, DEF_DATASIZE, pair):sendsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,DEF_DATASIZE,pair);
}

int con_read_tcp(con_t* con_obj,int_pair pair){

	return con_obj->is_ssl?readsome_ssl(con_obj->con_ssl,(char*)con_obj->tcp_data, DEF_DATASIZE, pair):readsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,DEF_DATASIZE,pair);
}

void ask_for_port(uint16_t* port,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		if(logging){
			perror("Conexão ao port mapper para pedir unica porta mal sucedida!\nSocket não pôde ser criada!\nAbortando\n");
		}
		raise(SIGINT);
		return;
	}
	if(init_addr(&addr, ent->hostname,ent->port)){
		if(logging){
			perror("Iniciacao de address para conectar ao port mapper para pedir unica porta unica mal sucedida. Abortando\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	int result_con=0;
	if((result_con=tryConnect(&tmp_socket,port_mapper_times_pair,&addr))<=0){
		if(logging){
			perror("Conexão ao port mapper para pedir unica porta mal sucedida! Abortando\n");
		}
		socket_close(&tmp_socket,result_con!=0);
		raise(SIGINT);
		return;
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_AWKWARD_JOIN_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("O port mapper nâo recebeu o nosso request!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	result=readsome(tmp_socket,(char*)port,sizeof((*port)),port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("Não conseguimos receber porta do port mapper!!!!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	if(logging){
		fprintf(logstream,"Recebemos porta do port_mapper!!!\n"
						"%hu\n",
						port[0]);
	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
        snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_JOIN_GOT_IT_STRING);
        result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
        if(result<=0){

                if(logging){
			fprintf(logstream,"Aviso de que já temos as portas não enviado!!!\nMensagem que devia ter sido enviada:\n%s\n",buff_for_ports);
		}
		close(tmp_socket);
		raise(SIGINT);
		return;
        }
        if(logging){
		fprintf(logstream,"Aviso de que já temos a porta enviado!!!\nMensagem que foi enviada:\n%s\n",buff_for_ports);
	}
	close(tmp_socket);
}

void ask_for_ports(ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(tmp_socket<0){
		if(logging){
			perror("Conexão ao port mapper para pedir unica porta mal sucedida!\nSocket não pôde ser criada!\nAbortando\n");
		}
		raise(SIGINT);
		return;
	}
	if(init_addr(&addr, ent->hostname,ent->port)){
		if(logging){
			perror("Iniciacao de address para conectar ao port mapper para pedir unica porta unica mal sucedida. Abortando\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	int result_con=0;
	if((result_con=tryConnect(&tmp_socket,port_mapper_times_pair,&addr))<=0){
		if(logging){
			perror("Conexão ao port mapper para pedir unica porta mal sucedida! Abortando\n");
		}
		socket_close(&tmp_socket,result_con!=0);
		raise(SIGINT);
		return;
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_JOIN_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("O port mapper nâo recebeu o nosso request!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	result=readsome(tmp_socket,(char*)attempted_port_arr,sizeof(port_array),port_mapper_times_pair);
	if(result<=0){
		if(logging){
			perror("Não conseguimos receber portas do port mapper!!!!!!\n");
		}
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	if(logging){
		fprintf(logstream,"Recebemos portas do port_mapper!!!\n"
						"%hu Delas!\n",
						attempted_port_arr[0]);
	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
        snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_JOIN_GOT_IT_STRING);
        result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
        if(result<=0){

                if(logging){
			fprintf(logstream,"Aviso de que já temos as portas não enviado!!!\nMensagem que devia ter sido enviada:\n%s\n",buff_for_ports);
		}
		close(tmp_socket);
		raise(SIGINT);
		return;
        }
        if(logging){
		fprintf(logstream,"Aviso de que já temos a porta enviado!!!\nMensagem que foi enviada:\n%s\n",buff_for_ports);
	}
	close(tmp_socket);
}


static void greet_server(con_t* con_obj, int_pair pair){

	if(con_obj->is_ssl){
		convert_server_con_to_ssl(&con_obj->con_ssl,con_obj->sockfd_tcp,pair);
	}
	else{
		con_obj->con_ssl=NULL;
	}

	char client_data[DEF_DATASIZE+1];
	memset(client_data,0,DEF_DATASIZE+1);
	con_read_tcp(con_obj,pair);
	sscanf((char*)con_obj->tcp_data,"%s",(char*)client_data);
	if(logging){
		fprintf(logstream,"tuplo recebido: (string) = (%s)\n",client_data);
	}
	clear_con_data(con_obj);

	int result=strs_are_strictly_equal(CON_STRING,client_data);

	if(result){

		if(logging){

			fprintf(logstream,"String de conexão errada recebida!\nRecebemos \"%s\" do cliente!\n",client_data);
		}
		raise(SIGINT);
	}
}

static void greet_client(con_t* con_obj,int_pair pair){

	if(con_obj->is_ssl){
		convert_client_con_to_ssl(&con_obj->con_ssl,con_obj->sockfd_tcp,pair);
	}
	else{
		con_obj->con_ssl=NULL;
	}

	snprintf((char*)con_obj->tcp_data,DEF_DATASIZE,"%s",CON_STRING);
	if(logging){
		fprintf(logstream,"String enviada %s\n",(char*)con_obj->tcp_data);
	}
	con_send_tcp(con_obj,pair);

	clear_con_data(con_obj);

}


void greet(con_t*con_obj,int_pair times_pair){

	switch(con_obj->type){
		case SERVER_C:
			greet_server(con_obj,times_pair);
			break;
		case CLIENT_C:
			greet_client(con_obj,times_pair);
			break;
		default:
			break;
	}

	if(logging){
		print_addr_aux("Addresss tcp de nos:",&con_obj->this_tcp_addr);
	}


}

void free_attempted_ports(uint16_t port_that_works,ip_cache_entry*ent){

	send_ports_back(ent,port_that_works);

}

void connection_attempt_circuit(int* socket_fd, void (*quit_handler)(int, void*),
					struct sockaddr_in* src_address,
					struct sockaddr_in* dst_address,
					ip_cache_entry* src_ent,
					ip_cache_entry* port_mapper_ent,
					int_pair con_times_pair,
					void* ptr){

        ask_for_ports(port_mapper_ent);
        int result_con=0;
	uint16_t curr_attempts=0;
        uint16_t limit_of_attempts=attempted_port_arr[0];
        while(curr_attempts<limit_of_attempts){
                (*socket_fd)= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
                if((*socket_fd)<0){

                        quit_handler(SIGINT,ptr);
                	return;
		}
                set_sock_reuseaddr(socket_fd,1);
                setNonBlocking(socket_fd);
                if(!attempted_port_arr[curr_attempts+1]||init_addr(src_address,src_ent->hostname,(attempted_port_arr[curr_attempts+1]))){
                        if(logging){
				perror("Não conseguimos inicializar address no client!!!\n");
                        }
			quit_handler(SIGINT,ptr);
			return;
                }
		curr_attempts++;
		if(!memcmp(src_address,dst_address,sizeof(struct sockaddr_in))){
			if(logging){

                                fprintf(logstream,"addresses src e dst iguais!\n");
                        	print_addr_aux("Este é o address:",src_address);
                        }
			quit_handler(SIGINT,ptr);
		}
		if(bind((*socket_fd),(struct sockaddr *)src_address,socklenvar[1])){
                        if(logging){
				perror("Não conseguimos dar bind na socket_fd do client!!!\n");
                        	print_addr_aux("Este é o address:",src_address);
                        }
			quit_handler(SIGINT,ptr);
                	return;
		}
                else{

                        if(logging){
				print_addr_aux("Bind com sucesso!!!:",src_address);
                        }
			setLinger(socket_fd,1,1);
                }
                if(!(result_con=tryConnect(socket_fd,con_times_pair,dst_address))){
                        if(logging){

                                fprintf(logstream,"Initiating forceful teardown!\nResult = %d\n\nsocket_fd fd; %d\n",result_con,(*socket_fd));
                        }
                        quit_handler(SIGINT,ptr);
			return;
                }
                else if(result_con>0){
                        free_attempted_ports(curr_attempts,port_mapper_ent);
                        return;
                }
                close((*socket_fd));
        }
	if(logging){
		perror("We tried all the ports that were given to us. None of them worked. Exiting...\n");
       	}
	quit_handler(SIGINT,ptr);
	return;
}
