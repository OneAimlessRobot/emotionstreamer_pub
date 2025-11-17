#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"
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


void send_port_back(uint16_t port,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(tmp_socket<0){
		perror("Criação de socket para conectar ao port mapper para devolver porta unica mal sucedida. Abortando\n");
		raise(SIGINT);
		return;
	}

	if(init_addr(&addr, ent->hostname,ent->port)){

		perror("Iniciacao de address para conectar ao port mapper para devolver porta unica mal sucedida. Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};

	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para devolver porta unica mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}

	
	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_AWKWARD_LEAVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos enviar o request de fecho de porta ao port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	snprintf(buff_for_ports,DEF_DATASIZE,"%hu",port);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos enviar a porta para fechar portas ao port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	else{
		close(tmp_socket);
	}

}

void close_con(con_t* con_obj,int RIGHT_NOW){
	
	if(con_obj->is_on){
		if(con_obj->sockfd_tcp>=0){
			socket_close(&(con_obj->sockfd_tcp),RIGHT_NOW);
		}
		con_obj->sockfd_tcp=-1;
		con_obj->is_on=0;
		if(logging){
			fprintf(logstream,"Fechamos conexão!!!!\n");
		}
	}
}
void init_con(con_t* con_obj,int sockfd_tcp,con_type type,uint16_t listen_port,ip_cache_entry *ent){

				prep_con(con_obj);
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

				con_obj->tcp_data_local_port=con_obj->listen_port=listen_port;
				memset(con_obj->tcp_data,0,DEF_DATASIZE+1);


}

int con_send_tcp(con_t* con_obj,int_pair pair){

	return sendsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,DEF_DATASIZE,pair);
}

int con_read_tcp(con_t* con_obj,int_pair pair){

	return readsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,DEF_DATASIZE,pair);
}

void ask_for_port(uint16_t* port,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		perror("Conexão ao port mapper para pedir unica porta mal sucedida!\nSocket não pôde ser criada!\nAbortando\n");
		raise(SIGINT);
		return;
	}
	if(init_addr(&addr, ent->hostname,ent->port)){
		perror("Iniciacao de address para conectar ao port mapper para pedir unica porta unica mal sucedida. Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para pedir unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_AWKWARD_JOIN_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber porta do port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	sscanf(buff_for_ports,"%hu",port);
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

void reserve_local_listening_port(uint16_t port_to_allocate,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		perror("Conexão ao port mapper para reservar unica porta mal sucedida!\nSocket não pôde ser criada!\nAbortando\n");
		raise(SIGINT);
		return;
	}

	if(init_addr(&addr, ent->hostname,ent->port)){
		perror("Iniciaçao de address para conexão ao port mapper para reservar unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para reservar unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	
	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_RESERVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado do nosso pedido de reserva no  mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	snprintf(buff_for_ports,DEF_DATASIZE,"%hu",port_to_allocate);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu a nossa porta!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado de reserva da nossa porta no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	int result_of_reserve=-1;
	sscanf(buff_for_ports,"%d",&result_of_reserve);
	if(!result_of_reserve){
		perror("Não foi possivel reservar porta de listening no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;


	}
	if(logging){
		fprintf(logstream,"Reserva feita!!!\n");
	}
	close(tmp_socket);


}
void unreserve_local_listening_port(uint16_t port_to_allocate,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		perror("Conexão ao port mapper para desreservar unica porta mal sucedida!\nSocket	 não pôde ser criada!\nAbortando\n");
		raise(SIGINT);
	}

	if(init_addr(&addr, ent->hostname,ent->port)){
		perror("Iniciaçao de address para conexão ao port mapper para desreservar unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para desreservar unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_UNRESERVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado do nosso pedido de desreserva no  mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	snprintf(buff_for_ports,DEF_DATASIZE,"%hu",port_to_allocate);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu a nossa porta!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;
	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado de desreserva da nossa porta no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;

	}
	int result_of_reserve=-1;
	sscanf(buff_for_ports,"%d",&result_of_reserve);
	if(!result_of_reserve){
		perror("Não foi possivel desreservar porta de listening no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);
		return;



	}
	if(logging){
		fprintf(logstream,"Desreserva feita!!!\n");
	}
	close(tmp_socket);

}


static void greet_server(con_t* con_obj, int_pair pair){

	char client_data[DEF_DATASIZE+1];
	memset(client_data,0,DEF_DATASIZE+1);
	con_read_tcp(con_obj,pair);
	sscanf((char*)con_obj->tcp_data,"%s %hu",(char*)client_data,&con_obj->tcp_data_peer_port);
	if(logging){
		fprintf(logstream,"tuplo recebido: (string, port) = (%s, %hu)\n",client_data,con_obj->tcp_data_peer_port);
	}
	clear_con_data(con_obj);
	
	int result=strs_are_strictly_equal(CON_STRING,client_data);
	
	if(result){

		if(logging){

			fprintf(logstream,"String de conexão errada recebida! Recebemos \"%s\" do cliente!",client_data);
		}
		raise(SIGINT);
	}
	else{
		snprintf((char*)con_obj->tcp_data,DEF_DATASIZE,"%hu",(uint16_t)(con_obj->tcp_data_local_port));

		if(logging){
			fprintf(logstream,"Portas enviadas: %s\n",(char*)con_obj->tcp_data);
		}
		con_send_tcp(con_obj,pair);

		clear_con_data(con_obj);



	}
}

static void greet_client(con_t* con_obj,int_pair pair){

	snprintf((char*)con_obj->tcp_data,DEF_DATASIZE,"%s %hu",CON_STRING,(uint16_t)(con_obj->tcp_data_local_port));
	if(logging){
		fprintf(logstream,"String enviada %s\n",(char*)con_obj->tcp_data);
	}
	con_send_tcp(con_obj,pair);

	clear_con_data(con_obj);

	con_read_tcp(con_obj,pair);

	if(logging){
		fprintf(logstream,"String recebida (portas do server) %s\n",(char*)con_obj->tcp_data);
	}
	sscanf((char*)con_obj->tcp_data,"%hu",&con_obj->tcp_data_peer_port);
	if(logging){
		fprintf(logstream,"Portas do client agora: %hu\n",con_obj->tcp_data_local_port);
	}
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
		print_addr_aux("Addresss tcp do peer:",&con_obj->peer_tcp_addr);

		print_addr_aux("Addresss tcp de nos:",&con_obj->this_tcp_addr);
	}


}

