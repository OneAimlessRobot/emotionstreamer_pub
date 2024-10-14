#include "../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../Includes/player.h"
#include "../Includes/client.h"
static struct sockaddr_in server_address_udp;
//static struct sockaddr_in client_address_udp;
static long int dataSize;
static int client_udp_sock;
static uint32_t server_udp_port;
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
	
	client_udp_sock= socket(AF_INET,SOCK_DGRAM,0);
	if(client_udp_sock==-1){
		raise(SIGINT);
	}

	//port exchange with server
	sendsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);
	
	memset(buff,0,DEF_DATASIZE);

	readsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);

	if(strs_are_strictly_equal(buff,ACK_STRING)){

		printf("Server rejeitou-nos\nString recebida: %s\n",buff);
		raise(SIGINT);

	}
	
	printf("Sucesso a cumprimentar!!!\n");
	
}
static void server_get_port(char buff[DEF_DATASIZE]){
	

	memset(buff,0,DEF_DATASIZE);
	snprintf(buff,DEF_DATASIZE-1,"%s",DEFAULT_PORT_STRING);

	sendsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);

	memset(buff,0,DEF_DATASIZE);

	readsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);

	sscanf(buff,"%u",&server_udp_port);
	printf("Buffer de porta: %s\nPorta: %u (do server)\nPorta: %u (nossa)\n",buff, server_udp_port,ntohl(server_udp_port));
	memset(buff,0,DEF_DATASIZE);

	//close(client_udp_sock);

	/*client_udp_sock= socket(AF_INET,SOCK_DGRAM,0);
	if(client_udp_sock==-1){
		raise(SIGINT);
	}*/
	server_address_udp.sin_port=ntohl(server_udp_port);

	print_addr_aux("Sucesso a obter porta!!!",&server_address_udp);

}
static void server_courtesy_dance(char buff[DEF_DATASIZE]){

	server_greet(buff);
	server_get_port(buff);

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

	char file_path[PATHSIZE]={0};
	
	char recvd_req_str_res[BUFFSIZE]={0};
	req_type_to_str(the_type,req_str_res);
	
	snprintf(file_path,PATHSIZE-1,"%s%s",curr_dir,file_name);
	
	snprintf(buff,DEF_DATASIZE-1,"%s",DEFAULT_CON_STRING);
	
	if(the_type==NA){
		printf(UNKNOWN_REQ);
		raise(SIGINT);
	}

	server_address_udp.sin_family=AF_INET;
	server_address_udp.sin_port= udp_s_port;

	struct hostent* hp= gethostbyname(s_hostname);

	memcpy(&(server_address_udp.sin_addr),hp->h_addr,hp->h_length);


	server_courtesy_dance(buff);
	print_addr_aux("End. do server agora:",&server_address_udp);
	printf(CONNECTION_ESTABLISHED_MSG,inet_ntoa(server_address_udp.sin_addr));
        printf("Enviamos request do tipo: %s\n",req_str_res);
	//enviar a string de request
        sendsome_udp(client_udp_sock,req_str_res,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);
        //Ler a resposta do server feita pq se alterna sempre reads com writes e quis enviar o request e o filename em separado
        readsome_udp(client_udp_sock,recvd_req_str_res,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);
	if(strs_are_strictly_equal(req_str_res,recvd_req_str_res)){

		printf("Tipos de request recebidos e enviados diferentes!\nEnviado: %s\nRecebido: %s\n",req_str_res,recvd_req_str_res);
		raise(SIGINT);
	}
	
        snprintf(file_name_container,PATHSIZE-1,"%s",file_name);
        //Enviando o filename
        sendsome_udp(client_udp_sock,file_name_container,PATHSIZE-1,CLIENT_DATA_TIMES_PAIR,&server_address_udp);





        //lendo o datasize
        readsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);
        printf("Data size buff: %s\n",buff);
        //Dizendo ao server "Hey! Recebemos datasize!!!! O q enviamos n é importante. So tem de ser "DEF_DATASIZE" Bytes em dimensão
        sendsome_udp(client_udp_sock,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,&server_address_udp);
        sscanf(buff,"%ld",&dataSize);
        printf(DOWNLOAD_SIZE_MSG,dataSize);
        if(!dataSize){
                printf(NO_SUCH_FILE_IN_SERVER);
                raise(SIGINT);
        }


	
	switch(the_type){

	case PLAY:
		if((fd= creat(file_path,0777))<0){
			perror(FILE_CREATION_ERROR);
			raise(SIGINT);
		}
		else{
			printf(CREATED_FILE_PATH_MSG, file_path);
		}
		break;
	case PEEK:
		fd=1;
		printf(CONTENT_PEEK_INCOMMING);
		break;
	default:
		printf(UNKNOWN_REQ);
		raise(SIGINT);
		break;
	}

	readalltofd_udp(client_udp_sock,fd,CLIENT_DATA_TIMES_PAIR,&server_address_udp);
	
	if(fd>2){
		close(fd);
		//player_init(file_path);
	}
	return 0;
}

















































/*
static void tryConnect(int* socket){
        int success=-1;
        int numOfTries=MAX_TRIES;


        while(success==-1&& numOfTries){
                printf(CONNECTION_ATTEMPT_NO_MSG,inet_ntoa(server_address_tcp.sin_addr),-numOfTries+MAX_TRIES+1);
                success=connect(*socket,(struct sockaddr*)&server_address_tcp,sizeof(server_address));
               int sockerr;
                socklen_t slen=sizeof(sockerr);
                getsockopt(*socket,SOL_SOCKET,SO_ERROR,(char*)&sockerr,&slen);
                numOfTries--;
                if(sockerr==EINPROGRESS){

                        fprintf(stderr,SOCK_ERROR_DUMP,strerror(errno),strerror(sockerr),*socket);
                        continue;

                }
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET((*socket),&wfds);

                struct timeval t;
                t.tv_sec=CLIENT_TIMEOUT_CON_SEC;
                t.tv_usec=CLIENT_TIMEOUT_CON_USEC;
                int iResult=select((*socket)+1,0,&wfds,0,&t);

                if(iResult>0&&!success&&((*socket)!=-1)){
			break;

                }
                fprintf(stderr,"Não foi possivel: %s\n",strerror(errno));
        }
        if(!numOfTries){
        printf(MAX_TRIES_HIT_ERROR,MAX_TRIES);
        raise(SIGINT);
        }

}*/
