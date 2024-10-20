#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"
#include "../Includes/configs.h"
#include "../Includes/sock_ops.h"
#include "../Includes/sockio_udp.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/connection.h"


void clear_con_data(con_t* con_obj){

	memset(con_obj->tcp_data,0,cfg_datasize+1);
	memset(con_obj->udp_data,0,cfg_datasize+1);
	memset(con_obj->ack_udp_data,0,cfg_datasize+1);


}

static void prep_con(con_t* con_obj){

	memset(con_obj,0,sizeof(con_t));

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

void init_con(con_t* con_obj,int sockfd_tcp,con_type type, buff_triple con_buffs){

				prep_con(con_obj);
				con_obj->is_on=1;
				con_obj->type=type;
                                con_obj->sockfd_tcp=sockfd_tcp;
                                getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_tcp_addr),socklenvar);
				getsockname(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->this_tcp_addr),socklenvar);


				con_obj->tcp_data=con_buffs[0];
				con_obj->udp_data=con_buffs[1];
				con_obj->ack_udp_data=con_buffs[2];

				memset(con_obj->tcp_data,0,cfg_datasize+1);
				memset(con_obj->udp_data,0,cfg_datasize+1);
				memset(con_obj->ack_udp_data,0,cfg_datasize+1);


}

int con_send_tcp(con_t* con_obj,int_pair pair){

	return sendsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,cfg_datasize,pair);
}

int con_read_tcp(con_t* con_obj,int_pair pair){

	return readsome(con_obj->sockfd_tcp,(char*)con_obj->tcp_data,cfg_datasize,pair);
}

int con_read_udp(con_t* con_obj, int_pair pair){

	return readsome_udp(con_obj->sockfd_udp,(char*)con_obj->udp_data,cfg_datasize,pair,&con_obj->peer_udp_addr);
}
int con_send_udp(con_t* con_obj,int_pair pair){

	return sendsome_udp(con_obj->sockfd_udp,(char*)con_obj->udp_data,cfg_datasize,pair,&con_obj->peer_udp_addr);
}

int con_read_udp_ack(con_t* con_obj, int_pair pair){

	return readsome_udp(con_obj->ack_sockfd_udp,(char*)con_obj->ack_udp_data,cfg_datasize,pair,&con_obj->peer_udp_ack_addr);
}
int con_send_udp_ack(con_t* con_obj,int_pair pair){

	return sendsome_udp(con_obj->ack_sockfd_udp,(char*)con_obj->ack_udp_data,cfg_datasize,pair,&con_obj->peer_udp_ack_addr);
}

static void set_up_peer_udp_socks(con_t* con_obj, int start_udp_port){

	getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_udp_addr),socklenvar);
	con_obj->peer_udp_addr.sin_port=start_udp_port;
	getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_udp_ack_addr),socklenvar);
	con_obj->peer_udp_ack_addr.sin_port=start_udp_port+1;


}

static void set_up_local_udp_socks(con_t* con_obj,int curr_port){


	
	con_obj->sockfd_udp= socket(AF_INET,SOCK_DGRAM,0);
        if(con_obj->sockfd_udp==-1){
                raise(SIGINT);
        }
	con_obj->ack_sockfd_udp= socket(AF_INET,SOCK_DGRAM,0);
        if(con_obj->sockfd_udp==-1){
                raise(SIGINT);
        }

	getsockname(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->this_udp_addr),socklenvar);
	con_obj->this_udp_addr.sin_port=curr_port+1;
        int bind_result=bind(con_obj->sockfd_udp,(struct sockaddr*) &(con_obj->this_udp_addr),*socklenvar);
	
	if(bind_result){
		
		perror("Erro no em bind da socket udp de dados no modulo de conexão!\n");
		close_con(con_obj);
		exit(-1);

	}
	getsockname(con_obj->sockfd_udp, (struct sockaddr*)&(con_obj->this_udp_ack_addr),socklenvar);
	con_obj->this_udp_ack_addr.sin_port=curr_port+2;
        bind_result=bind(con_obj->ack_sockfd_udp,(struct sockaddr*) &(con_obj->this_udp_ack_addr),*socklenvar);
	if(bind_result){

		perror("Erro no em bind da socket udp de acknowledgements modulo de conexão!\n");
		close_con(con_obj);
		exit(-1);

	}
}

static void greet_server(con_t* con_obj, int_pair pair,int curr_port){

 	char client_data[cfg_datasize+1];
	memset(client_data,0,cfg_datasize+1);
	uint16_t port_udp,ack_port_udp;
	con_read_tcp(con_obj,pair);

	sscanf((char*)con_obj->tcp_data,"%s %hu %hu",(char*)client_data,&port_udp,&ack_port_udp);

	printf("Triplo recebido: (string, port, port) = (%s, %hu, %hu)\n",client_data,port_udp,ack_port_udp);
	clear_con_data(con_obj);

	int result=strs_are_strictly_equal(CON_STRING,client_data);
	int port=result ? 0: curr_port;
	snprintf((char*)con_obj->tcp_data,cfg_datasize,"%hu %hu %hu",port, port+1,port+2);
	
	if(!port){
		close_con(con_obj);
		exit(-1);

	}
	printf("Portas enviadas: %hu, %hu, %hu\n",port, port+1,port+2);

	con_send_tcp(con_obj,pair);

	clear_con_data(con_obj);

	set_up_local_udp_socks(con_obj,curr_port);
	set_up_peer_udp_socks(con_obj,port_udp);

	printf("Server greet sucesfull; Portas recebidas: %hu, %hu\n",port_udp, ack_port_udp);


}

static void greet_client(con_t* con_obj,int_pair pair,int curr_port){
	
	
	uint16_t port_tcp,port_udp,ack_port_udp;
	snprintf((char*)con_obj->tcp_data,cfg_datasize,"%s %hu %hu",CON_STRING,curr_port+1,curr_port+2);

	printf("String enviada %s\n",(char*)con_obj->tcp_data);

	con_send_tcp(con_obj,pair);

	clear_con_data(con_obj);

	con_read_tcp(con_obj,pair);

	printf("String recebida %s\n",(char*)con_obj->tcp_data);
	
	sscanf((char*)con_obj->tcp_data,"%hu %hu %hu",&port_tcp,&port_udp,&ack_port_udp);

	printf("Portas agora: %hu %hu %hu\n",port_tcp,port_udp,ack_port_udp);

	if(port_udp==0){

		
		printf("Server rejeitou!\n");
		close_con(con_obj);
		exit(-1);

	}

	set_up_local_udp_socks(con_obj,curr_port);
	set_up_peer_udp_socks(con_obj,port_udp);

	printf("Client greet sucessful: Portas recebidas: %hu %hu %hu\n",port_tcp,port_udp,ack_port_udp);
}


void greet(con_t*con_obj,int curr_port){

	switch(con_obj->type){
		case SERVER_C:
			greet_server(con_obj,server_data_times_pair,curr_port);
			break;
		case CLIENT_C:
			greet_client(con_obj,client_data_times_pair,curr_port);
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

