#include "../Includes/preprocessor.h"
/*#include "../../player/SDL2/include/SDL.h"
#include "../../player/SDL2/include/SDL_mixer.h"
#include "../../player/SDL2/include/SDL_rwops.h"
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../Includes/streamer_client.h"
#include "../Includes/client.h"
static struct sockaddr_in server_ip_address;
static long int dataSize;
static int client_udp_sock;
static int client_tcp_sock;
static uint32_t server_tcp_port;
static int fd;



static void sigint_handler(int signal){

	close(client_udp_sock);
	close(fd);
	printf("Saimos com erro. Vamos ver errno:%s\n",strerror(errno));
	exit(-1);
}


static void sigpipe_handler(int signal){
	sigint_handler(signal);
}

static void server_greet(char buff[DEF_DATASIZE]){
	
	
	
	//port exchange with server
	sendsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_ip_address);
	
	memset(buff,0,DEF_DATASIZE);

	readsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_ip_address);

	if(strs_are_strictly_equal(buff,ACK_STRING)){

		printf("Server rejeitou-nos\nString recebida: %s\n",buff);
		raise(SIGINT);

	}
	
	printf("Sucesso a cumprimentar!!!\n");
	
}

static void server_get_port(char buff[DEF_DATASIZE]){
	

	memset(buff,0,DEF_DATASIZE);
	snprintf(buff,DEF_DATASIZE-1,"%s",DEFAULT_PORT_STRING);

	sendsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_ip_address);

	memset(buff,0,DEF_DATASIZE);

	readsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_ip_address);

	sscanf(buff,"%u",&server_tcp_port);
	printf("Buffer de porta: %s\nPorta: %u (do server)\nPorta: %u (nossa)\n",buff, server_tcp_port,ntohl(server_tcp_port));
	memset(buff,0,DEF_DATASIZE);

	//close(client_udp_sock);

	/*client_udp_sock= socket(AF_INET,SOCK_DGRAM,0);
	if(client_udp_sock==-1){
		raise(SIGINT);
	}*/
	server_ip_address.sin_port=ntohl(server_tcp_port);

	print_addr_aux("Sucesso a obter porta!!!",&server_ip_address);

}
static void server_courtesy_dance(char buff[DEF_DATASIZE]){

	server_greet(buff);
	server_get_port(buff);

}

static void set_up_socks(char buff[DEF_DATASIZE]){



	client_udp_sock= socket(AF_INET,SOCK_DGRAM,0);
	if(client_udp_sock==-1){
		raise(SIGINT);
	}

	client_tcp_sock= socket(AF_INET,SOCK_STREAM,0);
	if(client_tcp_sock==-1){
		raise(SIGINT);
	}

	server_courtesy_dance(buff);

	bind(client_udp_sock,(struct sockaddr*)(&server_ip_address),*socklenvar);

	print_addr_aux("End. do server agora:",&server_ip_address);


	tryConnect(&client_tcp_sock,CLIENT_CON_TIMES_PAIR,&server_ip_address);

	printf(CONNECTION_ESTABLISHED_MSG,inet_ntoa(server_ip_address.sin_addr));

}
//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,uint32_t udp_s_port, char* s_hostname){
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	
	logstream=stderr;
	logging=1;
	
	req_type the_type= str_to_req_type(req_field);
	//receber e armazenar dados recebidos
	char buff[DEF_DATASIZE]={0};
	char file_name_container[PATHSIZE]={0};
	
	char req_str_res[BUFFSIZE]={0};

	char file_path[PATHSIZE*2]={0};
	
	char recvd_req_str_res[BUFFSIZE]={0};
	req_type_to_str(the_type,req_str_res);
	
	snprintf(file_path,PATHSIZE+1,"%s%s",curr_dir,file_name);
	
	snprintf(buff,DEF_DATASIZE-1,"%s",DEFAULT_CON_STRING);
	
	if(the_type==NA){
		printf(UNKNOWN_REQ);
		raise(SIGINT);
	}

	init_addr(&server_ip_address,s_hostname,udp_s_port);
	
	set_up_socks(buff);

        printf("Enviamos request do tipo: %s\n",req_str_res);
	//enviar a string de request
        sendsome(client_tcp_sock,req_str_res,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR);
        //Ler a resposta do server feita pq se alterna sempre reads com writes e quis enviar o request e o filename em separado
        readsome(client_tcp_sock,recvd_req_str_res,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR);
	if(strs_are_strictly_equal(req_str_res,recvd_req_str_res)){

		printf("Tipos de request recebidos e enviados diferentes!\nEnviado: %s\nRecebido: %s\n",req_str_res,recvd_req_str_res);
		raise(SIGINT);
	}
	
        snprintf(file_name_container,PATHSIZE-1,"%s",file_name);
        //Enviando o filename
        sendsome(client_tcp_sock,file_name_container,PATHSIZE-1,CLIENT_DATA_TIMES_PAIR);





        //lendo o datasize
        readsome(client_tcp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR);
        printf("Data size buff: %s\n",buff);
        //Dizendo ao server "Hey! Recebemos datasize!!!! O q enviamos n é importante. So tem de ser "DEF_DATASIZE" Bytes em dimensão
        sendsome(client_tcp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR);
        sscanf(buff,"%ld",&dataSize);
        printf(DOWNLOAD_SIZE_MSG,dataSize);
        if(!dataSize){
                printf(NO_SUCH_FILE_IN_SERVER);
                raise(SIGINT);
        }



	switch(the_type){

	case PLAY:
		//O SERVER É quem começa a enviar!!!!!!
		//SERVER COMECA A ENVIAR A STREAM A PARTIR DAQUI!!!!!!!!
		//readalltofd(client_tcp_sock,fd,CLIENT_DATA_TIMES_PAIR);
		printf("Download feito! Tocando...\n");
		player_init_stream(client_tcp_sock);
		break;
	case PEEK:
		fd=1;
		printf(CONTENT_PEEK_INCOMMING);
		readalltofd(client_tcp_sock,fd,CLIENT_DATA_TIMES_PAIR);
		
		break;
	default:
		printf(UNKNOWN_REQ);
		break;
	}
	raise(SIGINT);
	return 0;
}

