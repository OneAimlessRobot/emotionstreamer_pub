#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"
#include "../Includes/sock_ops.h"
#include "../Includes/sockio_udp.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/ip_cache_file.h"
#include "../Includes/connection.h"
#include "../../port_mapper/Includes/mapper.h"
#include "../Includes/generalized_config.h"


void clear_con_data(con_t* con_obj){

	memset(con_obj->tcp_data,0,DEF_DATASIZE+1);
	memset(con_obj->udp_data,0,DEF_DATASIZE+1);
	memset(con_obj->ack_udp_data,0,DEF_DATASIZE+1);


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
	}

	init_addr(&addr, ent->hostname,ent->port);
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};

	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para devolver porta unica mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		//return;
	}

	
	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_AWKWARD_LEAVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos enviar o request de fecho de porta ao port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	snprintf(buff_for_ports,DEF_DATASIZE,"%hu",port);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos enviar a porta para fechar portas ao port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	else{
		close(tmp_socket);
	}

}
void send_ports_back(con_t* obj){
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(tmp_socket<0){
		perror("Criação de socket para conectar ao port mapper para devolver multiplas portas mal sucedida. Abortando\n");
		close_con(obj);
		raise(SIGINT);
	}

	init_addr(&obj->port_mapper_addr, obj->port_mapper_entry.hostname,obj->port_mapper_entry.port);
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};

	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&obj->port_mapper_addr)){
		perror("Conexão ao port mapper para devolver multiplas portas mal sucedida! Abortando\n");
		close(tmp_socket);
		close_con(obj);
		raise(SIGINT);
		//return;
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_LEAVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		close_con(obj);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos enviar o request de fecho de portas ao port mapper!!!!!!\n");
		close(tmp_socket);
		close_con(obj);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	snprintf(buff_for_ports,DEF_DATASIZE,"%hu %hu",obj->udp_data_local_port,obj->udp_ack_local_port);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos enviar as portas para fechar portas ao port mapper!!!!!!\n");
		close(tmp_socket);
		close_con(obj);
		raise(SIGINT);

	}
	else{
		close(tmp_socket);
	}

}
void close_con(con_t* con_obj){
	
	if(con_obj->is_on){
		close(con_obj->sockfd_tcp);
		close(con_obj->sockfd_udp);
		close(con_obj->ack_sockfd_udp);
		con_obj->is_on=0;
		printf("Fechamos conexão!!!!\n");
	}
}

void drop_peer_con(con_t* con_obj){

		close(con_obj->sockfd_udp);
		close(con_obj->ack_sockfd_udp);
		printf("Largamos peer!!!!\n");
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
				con_obj->udp_data_local_port=0;
				con_obj->udp_ack_local_port=0;

				con_obj->udp_data_peer_port=0;
				con_obj->udp_ack_peer_port=0;

				con_obj->tcp_data_local_port=con_obj->listen_port=listen_port;
				memset(con_obj->tcp_data,0,DEF_DATASIZE+1);
				memset(con_obj->udp_data,0,DEF_DATASIZE+1);
				memset(con_obj->ack_udp_data,0,DEF_DATASIZE+1);


}

int con_send_tcp(con_t* con_obj,int_pair pair){

	return sendsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,DEF_DATASIZE,pair);
}

int con_read_tcp(con_t* con_obj,int_pair pair){

	return readsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,DEF_DATASIZE,pair);
}

int con_read_udp(con_t* con_obj, int_pair pair){

	return readsome_udp(con_obj->sockfd_udp,(char*)con_obj->udp_data,DEF_DATASIZE,pair,&con_obj->peer_udp_addr);
}
int con_send_udp(con_t* con_obj,int_pair pair){

	return sendsome_udp(con_obj->sockfd_udp,(char*)con_obj->udp_data,DEF_DATASIZE,pair,&con_obj->peer_udp_addr);
}

int con_read_udp_ack(con_t* con_obj, int_pair pair){

	return readsome_udp(con_obj->ack_sockfd_udp,(char*)con_obj->ack_udp_data,DEF_DATASIZE,pair,&con_obj->peer_udp_ack_addr);
}
int con_send_udp_ack(con_t* con_obj,int_pair pair){

	return sendsome_udp(con_obj->ack_sockfd_udp,(char*)con_obj->ack_udp_data,DEF_DATASIZE,pair,&con_obj->peer_udp_ack_addr);
}

void ask_for_ports(con_t* obj){
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		perror("Conexão ao port mapper para pedir multiplas portas mal sucedida!\nSocket não pôde ser criada!\nAbortando\n");
		close_con(obj);
		raise(SIGINT);
	}
	print_ip_cache_entry(stdout,&obj->port_mapper_entry);
	init_addr(&obj->port_mapper_addr, obj->port_mapper_entry.hostname,obj->port_mapper_entry.port);
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&obj->port_mapper_addr)){
		perror("Conexão ao port mapper para pedir multiplas portas mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
		//return;
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_JOIN_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber portas do port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	sscanf(buff_for_ports,"%hu %hu",&obj->udp_data_local_port,&obj->udp_ack_local_port);
	printf("Recebemos portas do port_mapper!!!\n"
						"Porta de dados udp: %hu\n"
						"Porta de acks udp: %hu\n",
						obj->udp_data_local_port,
						obj->udp_ack_local_port);

	memset(buff_for_ports,0,DEF_DATASIZE+1);
        snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_JOIN_GOT_IT_STRING);
        result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
        if(result<=0){

                printf("Aviso de que já temos as portas não enviado!!!\nMensagem que devia ter sido enviada:\n%s\n",buff_for_ports);
		close(tmp_socket);
		raise(SIGINT);
        }
        else{

                printf("Aviso de que já temos as portas enviado!!!\nMensagem que foi enviada:\n%s\n",buff_for_ports);
		close(tmp_socket);

	  }
}
void ask_for_port(uint16_t* port,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		perror("Conexão ao port mapper para pedir unica porta mal sucedida!\nSocket não pôde ser criada!\nAbortando\n");
		raise(SIGINT);
	}
	init_addr(&addr, ent->hostname,ent->port);
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para pedir unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		raise(SIGINT);
	}

	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_AWKWARD_JOIN_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber porta do port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	sscanf(buff_for_ports,"%hu",port);
	printf("Recebemos porta do port_mapper!!!\n"
						"%hu\n",
						port[0]);

	memset(buff_for_ports,0,DEF_DATASIZE+1);
        snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_JOIN_GOT_IT_STRING);
        result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
        if(result<=0){

                printf("Aviso de que já temos as portas não enviado!!!\nMensagem que devia ter sido enviada:\n%s\n",buff_for_ports);
		close(tmp_socket);
		raise(SIGINT);
        }
        else{

                printf("Aviso de que já temos a porta enviado!!!\nMensagem que foi enviada:\n%s\n",buff_for_ports);
		close(tmp_socket);

	  }
}

void reserve_local_listening_port(uint16_t port_to_allocate,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		perror("Conexão ao port mapper para reservar unica porta mal sucedida!\nSocket não pôde ser criada!\nAbortando\n");
		raise(SIGINT);
	}

	init_addr(&addr, ent->hostname,ent->port);
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para reservar unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		return;

	}
	
	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_RESERVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado do nosso pedido de reserva no  mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	snprintf(buff_for_ports,DEF_DATASIZE,"%hu",port_to_allocate);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu a nossa porta!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado de reserva da nossa porta no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	int result_of_reserve=-1;
	sscanf(buff_for_ports,"%d",&result_of_reserve);
	if(!result_of_reserve){
		perror("Não foi possivel reservar porta de listening no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);



	}
	printf("Reserva feita!!!\n");
	close(tmp_socket);


}
void unreserve_local_listening_port(uint16_t port_to_allocate,ip_cache_entry* ent){
	struct sockaddr_in addr={0};
	int tmp_socket= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tmp_socket<0){
		perror("Conexão ao port mapper para desreservar unica porta mal sucedida!\nSocket	 não pôde ser criada!\nAbortando\n");
		raise(SIGINT);
	}

	init_addr(&addr, ent->hostname,ent->port);
	int result=-1;
	char buff_for_ports[DEF_DATASIZE+1]={0};
	
	if(!tryConnect(&tmp_socket,port_mapper_times_pair,&addr)){
		perror("Conexão ao port mapper para desreservar unica porta mal sucedida! Abortando\n");
		close(tmp_socket);
		return;
	}
	snprintf(buff_for_ports,DEF_DATASIZE,"%s",PORT_MAPPER_UNRESERVE_STRING);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu o nosso request!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado do nosso pedido de desreserva no  mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	snprintf(buff_for_ports,DEF_DATASIZE,"%hu",port_to_allocate);
	result=sendsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("O port mapper nâo recebeu a nossa porta!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	memset(buff_for_ports,0,DEF_DATASIZE+1);
	result=readsome(tmp_socket,buff_for_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		perror("Não conseguimos receber resultado de desreserva da nossa porta no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);

	}
	int result_of_reserve=-1;
	sscanf(buff_for_ports,"%d",&result_of_reserve);
	if(!result_of_reserve){
		perror("Não foi possivel desreservar porta de listening no port mapper!!!!!!\n");
		close(tmp_socket);
		raise(SIGINT);



	}
	else{
		printf("Desreserva feita!!!\n");
		close(tmp_socket);
	}

}
static void set_up_peer_udp_socks(con_t* con_obj){
	
	socklen_t socklen_in=sizeof(struct sockaddr_in);
	//socklen_t socklen=sizeof(struct sockaddr);

	getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_udp_addr),&socklen_in);
	con_obj->peer_udp_addr.sin_port=htons(con_obj->udp_data_peer_port);

	socklen_in=sizeof(struct sockaddr_in);
	//socklen=sizeof(struct sockaddr);

	getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_udp_ack_addr),&socklen_in);
	con_obj->peer_udp_ack_addr.sin_port=htons(con_obj->udp_ack_peer_port);


}

static void set_up_local_udp_socks(con_t* con_obj){


	con_obj->sockfd_udp= socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	
    int ptr=1;
        if(con_obj->sockfd_udp==-1){
                raise(SIGINT);
        }
	
	setsockopt(con_obj->sockfd_udp,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
	con_obj->ack_sockfd_udp= socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(con_obj->ack_sockfd_udp==-1){
                raise(SIGINT);
        }
	
	setsockopt(con_obj->ack_sockfd_udp,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
	//setNonBlocking(con_obj->sockfd_udp);
	//setNonBlocking(con_obj->ack_sockfd_udp);

	
	socklen_t socklen_in=sizeof(struct sockaddr_in);
	//socklen_t socklen;=sizeof(struct sockaddr);

	getsockname(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->this_udp_addr),&socklen_in);
	con_obj->this_udp_addr.sin_port=htons(con_obj->udp_data_local_port);

        int bind_result=bind(con_obj->sockfd_udp,(struct sockaddr*) &(con_obj->this_udp_addr),socklenvar[0]);

	if(bind_result){
		perror("Erro no em bind da socket udp de dados no modulo de conexão!\n");
		print_addr_aux("",&con_obj->this_udp_addr);
		raise(SIGINT);

	}
	else{
		print_addr_aux("Demos bind em socket local de data UDP!!!!\nO address e:",&con_obj->this_udp_addr);
	}

	socklen_in=sizeof(struct sockaddr_in);
	/*socklen=sizeof(struct sockaddr);*/

	getsockname(con_obj->sockfd_udp, (struct sockaddr*)&(con_obj->this_udp_ack_addr),&socklen_in);

	con_obj->this_udp_ack_addr.sin_port=htons(con_obj->udp_ack_local_port);
        bind_result=bind(con_obj->ack_sockfd_udp,(struct sockaddr*) &(con_obj->this_udp_ack_addr),socklenvar[1]);

	if(bind_result){

		perror("Erro no em bind da socket udp de acknowledgements modulo de conexão!\n");
		print_addr_aux("",&con_obj->this_udp_ack_addr);
		raise(SIGINT);

	}
	else{

		print_addr_aux("Demos bind em socket local de acks UDP!!!\nO address e:",&con_obj->this_udp_ack_addr);
	}
}

static void greet_server(con_t* con_obj, int_pair pair,int_pair holepunching_times_pair){

	char client_data[DEF_DATASIZE+1];
	memset(client_data,0,DEF_DATASIZE+1);
	con_read_tcp(con_obj,pair);

	sscanf((char*)con_obj->tcp_data,"%s %hu %hu %hu",(char*)client_data,&con_obj->tcp_data_peer_port,&con_obj->udp_data_peer_port,&con_obj->udp_ack_peer_port);

	printf("Quatruplo recebido: (string, port, port, port) = (%s, %hu, %hu, %hu)\n",client_data,con_obj->tcp_data_peer_port,con_obj->udp_data_peer_port,con_obj->udp_ack_peer_port);
	clear_con_data(con_obj);
	
	int result=strs_are_strictly_equal(CON_STRING,client_data);
	
	if(result){

		printf("String de conexão errada recebida! Recebemos \"%s\" do cliente!",client_data);
		raise(SIGINT);
	}
	set_up_peer_udp_socks(con_obj);

	ask_for_ports(con_obj);

	snprintf((char*)con_obj->tcp_data,DEF_DATASIZE,"%hu %hu %hu",(uint16_t)(con_obj->tcp_data_local_port),(uint16_t)(con_obj->udp_data_local_port),(uint16_t)(con_obj->udp_ack_local_port));

	printf("Portas enviadas: %s\n",(char*)con_obj->tcp_data);

	con_send_tcp(con_obj,pair);

	clear_con_data(con_obj);

	set_up_local_udp_socks(con_obj);
	
	printf("Server greet sucesfull so far!\nWaiting for client to initiate hole punching routines!\n");
	printf("A receber UDP primeiro pela pipeline de dados!\n");
	clear_con_data(con_obj);
	snprintf((char*)con_obj->udp_data,DEF_DATASIZE,"Hole punching 1: reply");
	con_send_udp(con_obj,holepunching_times_pair);
	printf("Ok....\nOK rápido, rápido!!\nA receber UDP, mas agora pela pipeline de acknowledgments!\n");
	snprintf((char*)con_obj->ack_udp_data,DEF_DATASIZE,"Hole punching 2: reply");
	con_send_udp_ack(con_obj,holepunching_times_pair);
	printf("Tudo Enviado! Esperando resposta!\n");
	clear_con_data(con_obj);
	if(con_read_udp(con_obj,holepunching_times_pair)>0){
		printf("A resposta foi '%s'\nUDP holepunching recebido!\nEnviando resposta!\n",con_obj->udp_data);
	
	}
	else{
		printf("OOofff... reply de holepunching não recebida!!\n");
	

	}
	if(con_read_udp_ack(con_obj,holepunching_times_pair)>0){
		printf("A resposta foi '%s'\nUDP holepunching recebido na pipeline de acknowledgements!\nEnviando resposta!\n",con_obj->ack_udp_data);
	
	}
	else{
		printf("OOofff... reply de holepunching nos acks não recebida!!\n");
	

	}
	
}

static void greet_client(con_t* con_obj,int_pair pair,int_pair holepunching_times_pair){
	
	
 	
	ask_for_ports(con_obj);
	set_up_local_udp_socks(con_obj);

	snprintf((char*)con_obj->tcp_data,DEF_DATASIZE,"%s %hu %hu %hu",CON_STRING,(uint16_t)(con_obj->tcp_data_local_port),(uint16_t)(con_obj->udp_data_local_port),(uint16_t)(con_obj->udp_ack_local_port));

	printf("String enviada %s\n",(char*)con_obj->tcp_data);

	con_send_tcp(con_obj,pair);

	clear_con_data(con_obj);

	con_read_tcp(con_obj,pair);

	printf("String recebida (portas do server) %s\n",(char*)con_obj->tcp_data);
	
	sscanf((char*)con_obj->tcp_data,"%hu %hu %hu",&con_obj->tcp_data_peer_port,&con_obj->udp_data_peer_port,&con_obj->udp_ack_peer_port);

	printf("Portas do client agora: %hu %hu %hu\n",con_obj->tcp_data_local_port,con_obj->udp_data_local_port,con_obj->udp_ack_local_port);


	set_up_peer_udp_socks(con_obj);

	printf("Client greet sucessful so far!\nBeginning exaustive hole Punching routines!\n");
	printf("A enviar UDP primeiro pela pipeline de dados!\n");
	clear_con_data(con_obj);
	snprintf((char*)con_obj->udp_data,DEF_DATASIZE,"Hole punching 1");
	con_send_udp(con_obj,holepunching_times_pair);
	printf("Alright!\nrápido, rápido!!\nA enviar o furo pela pipeline UDP de acknowledgements!\n");
	snprintf((char*)con_obj->ack_udp_data,DEF_DATASIZE,"Hole punching 2");
	con_send_udp_ack(con_obj,holepunching_times_pair);
	printf("Tudo Enviado! Esperando resposta!\n");
	clear_con_data(con_obj);
	if(con_read_udp(con_obj,holepunching_times_pair)>0){
		printf("O que recebemos foi: '%s'\nRecebido!\nOkay! Agora vamos furar na pipeline de acknowledgements!\n",con_obj->udp_data);
	}
	else{
		printf("OOofff... reply de holepunching nos acks não recebida!!\n");

	}
	if(con_read_udp_ack(con_obj,holepunching_times_pair)>0){
		printf("O que recebemos foi: '%s'\nRecebido!\nOkay! Agora vamos furar na pipeline de acknowledgements!\n",con_obj->ack_udp_data);
	}
	else{
		printf("OOofff... reply de holepunching nos acks não recebida!!\n");

	}
}


void greet(con_t*con_obj,int_pair times_pair,int_pair holepunching_times_pair){

	switch(con_obj->type){
		case SERVER_C:
			greet_server(con_obj,times_pair,holepunching_times_pair);
			break;
		case CLIENT_C:
			greet_client(con_obj,times_pair,holepunching_times_pair);
			break;
		default:
			break;
	}

	print_addr_aux("Addresss tcp do peer:",&con_obj->peer_tcp_addr);

	print_addr_aux("Addresss tcp de nos:",&con_obj->this_tcp_addr);

	print_addr_aux("Addresss udp de dados do peer:",&con_obj->peer_udp_addr);

	print_addr_aux("Addresss udp de dados de nos:",&con_obj->this_udp_addr);

	print_addr_aux("Addresss udp de ack do peer:",&con_obj->peer_udp_ack_addr);

	print_addr_aux("Addresss udp de ack de nos:",&con_obj->this_udp_ack_addr);
	
	


}

