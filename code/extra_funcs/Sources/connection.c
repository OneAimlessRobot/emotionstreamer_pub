#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"
#include "../Includes/sock_ops.h"
#include "../Includes/sockio_udp.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/connection.h"

void clear_con_data(con_t* con_obj){

	memset(con_obj->data,0,DEF_DATASIZE+1);


}

static void prep_con(con_t* con_obj){

	memset(con_obj,0,sizeof(con_t));

}


void close_con(con_t* con_obj){

	close(con_obj->sockfd_tcp);
	close(con_obj->sockfd_udp);

}

void init_con(con_t* con_obj,int sockfd_tcp,con_type type){

				prep_con(con_obj);
				con_obj->is_on=1;
				con_obj->type=type;
                                con_obj->sockfd_tcp=sockfd_tcp;
                                getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_tcp_addr),socklenvar);
				getsockname(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->this_tcp_addr),socklenvar);
				
				getpeername(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->peer_udp_addr),socklenvar);
				memset(con_obj->data,0,DEF_DATASIZE+1);


}

int con_send_tcp(con_t* con_obj,int_pair pair){

	return sendsome(con_obj->sockfd_tcp,(char*)con_obj->data,DEF_DATASIZE,pair);
}

int con_send_udp(con_t* con_obj,int_pair pair){

	return sendsome_udp(con_obj->sockfd_udp,(char*)con_obj->data,DEF_DATASIZE,pair,&con_obj->peer_udp_addr);
}
int con_read_tcp(con_t* con_obj,int_pair pair){

	return readsome(con_obj->sockfd_tcp,(char*)con_obj->data,DEF_DATASIZE,pair);
}

int con_read_udp(con_t* con_obj, int_pair pair){

	return readsome_udp(con_obj->sockfd_udp,(char*)con_obj->data,DEF_DATASIZE,pair,&con_obj->peer_udp_addr);
}


static void set_up_udp_sock(con_t* con_obj){


	
	con_obj->sockfd_udp= socket(AF_INET,SOCK_DGRAM,0);
        if(con_obj->sockfd_udp==-1){
                raise(SIGINT);
        }

	getsockname(con_obj->sockfd_tcp, (struct sockaddr*)&(con_obj->this_udp_addr),socklenvar);
	con_obj->this_udp_addr.sin_port=con_obj->this_tcp_addr.sin_port+1;
	
	
        int bind_result=bind(con_obj->sockfd_udp,(struct sockaddr*) &(con_obj->this_udp_addr),*socklenvar);
	if(bind_result){
		
		perror("Erro no em bind no modulo de conexão!\n");
		close_con(con_obj);
		exit(-1);

	}
	
}

static void greet_server(con_t* con_obj){

 	char client_data[DEF_DATASIZE+1]={0};
	con_read_tcp(con_obj,SERVER_DATA_TIMES_PAIR);

	sscanf((char*)con_obj->data,"%s %hu",(char*)client_data,&(con_obj->peer_udp_addr.sin_port));

	printf("Par recebido: (string, port) = (%s, %hu)\n",client_data,con_obj->peer_udp_addr.sin_port);
	clear_con_data(con_obj);

	int result=strs_are_strictly_equal(CON_STRING,client_data);

	snprintf((char*)con_obj->data,DEF_DATASIZE,"%hu",(result=result ? 0: con_obj->this_tcp_addr.sin_port+1));

	con_send_tcp(con_obj,SERVER_DATA_TIMES_PAIR);

	if(!result){
		printf("Cliente rejeitado! Porta enviada ao cliente: %hu\n",result);
		close_con(con_obj);
		exit(-1);

	}
	clear_con_data(con_obj);

	set_up_udp_sock(con_obj);

	printf("Server greet sucessful! Porta enviada ao cliente: %hu\n",result);

}

static void greet_client(con_t* con_obj){
	
	
	snprintf((char*)con_obj->data,DEF_DATASIZE,"%s %hu",CON_STRING,con_obj->this_tcp_addr.sin_port+1);

	con_send_tcp(con_obj,CLIENT_DATA_TIMES_PAIR);

	clear_con_data(con_obj);

	con_read_tcp(con_obj,CLIENT_DATA_TIMES_PAIR);

	sscanf((char*)con_obj->data,"%hu",&(con_obj->peer_udp_addr.sin_port));

	if(con_obj->peer_udp_addr.sin_port==0){

		
		printf("Server rejeitou!\n");
		close_con(con_obj);
		exit(-1);

	}

	set_up_udp_sock(con_obj);

	printf("Client greet sucessful!\n");
}


void greet(con_t*con_obj){

	switch(con_obj->type){
		case SERVER_C:
			greet_server(con_obj);
			break;
		case CLIENT_C:
			greet_client(con_obj);
			break;
		default:
			break;
	}

	print_addr_aux("Addresss tcp do peer:",&con_obj->peer_tcp_addr);

	print_addr_aux("Addresss tcp de nos:",&con_obj->this_tcp_addr);

	print_addr_aux("Addresss udp do peer:",&con_obj->peer_udp_addr);

	print_addr_aux("Addresss udp de nos:",&con_obj->this_udp_addr);


}

