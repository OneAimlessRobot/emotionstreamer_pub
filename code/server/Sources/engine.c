#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/connection.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../Includes/engine.h"

server_state state;
static uint32_t curr_port=0;

static void addr_comp_err_aux(char* prompt,struct sockaddr_in* addr1,struct sockaddr_in* addr2){

	printf("%s\nEndereço 1: \n%s Porta 1: %u\n\nEndereço 2: \n%s Porta 2: %u\n",prompt,inet_ntoa(addr1->sin_addr),addr1->sin_port,inet_ntoa(addr2->sin_addr),addr2->sin_port);
	

}
static void serverStop(int useless){

	close(state.server_sock_udp);
	state.server_is_on=0;
	exit(useless);
}
static void sigint_handler(int useless){

	printf("Interrupção no modulo do server!!!!\n");
	serverStop(useless);
}
static void sigpipe_handler(int useless){

	printf("Pipe partido no modulo do server!!!!\n");
	serverStop(useless);
}
static int port_exchange_client_greet(char buff[DEF_DATASIZE]){

	
	//printf("A trocar portas... Endereço: (é lixo)\n%s Porta: %u\n", inet_ntoa(state.client_udp_addr.sin_addr),state.client_udp_addr.sin_port);
	
	//printf("A trocar portas... Tamanho do endereco: %lu\n",sizeof(state.client_udp_addr));
	int create_socket=0;
	readsome_udp(state.server_sock_udp,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,(&(state.client_udp_addr)));
	
	print_addr_aux("Addresss do cliente:",&state.client_udp_addr);
	printf("Received con string: |%s|\n",buff);
	if(!strs_are_strictly_equal(buff,DEFAULT_CON_STRING)){

		memset(buff,0,DEF_DATASIZE);
		snprintf(buff,DEF_DATASIZE-1,"%s",ACK_STRING);
		sendsome_udp(state.server_sock_udp,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,(&(state.client_udp_addr)));
	
		memset(buff,0,DEF_DATASIZE);
		readsome_udp(state.server_sock_udp,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,(&(state.client_udp_addr)));


		printf("Received port string: |%s|\n",buff);
		if(!strs_are_strictly_equal(buff,DEFAULT_PORT_STRING)){
			curr_port++;
			create_socket=1;
			memset(buff,0,DEF_DATASIZE);
			snprintf(buff,DEF_DATASIZE-1,"%u",htonl(curr_port));
			printf("Porta a ser enviada: %u\nVersao nossa: %u\n",htonl(curr_port),curr_port);
			sendsome_udp(state.server_sock_udp,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR,(&(state.client_udp_addr)));

		}
	}
	else{

		memset(buff,0,DEF_DATASIZE);
		snprintf(buff,DEF_DATASIZE-1,"%s",NACK_STRING);
	}
	
	return create_socket;
}

static void con_accepting_loop(void){

		while(state.server_is_on){
		
			
			int iResult,
				create_socket,
				sock,
				pid=-1;
			char buff[DEF_DATASIZE]={0};
		
	                struct timeval tv;
	                tv.tv_sec=SERVER_TIMEOUT_CON_SEC;
	                tv.tv_usec=SERVER_TIMEOUT_CON_USEC;
		

	                FD_ZERO(&state.rdfds);
	                FD_SET(state.server_sock_udp,&state.rdfds);
			
	                iResult=select(state.server_sock_udp+1,&state.rdfds,(fd_set*)0,(fd_set*)0,&tv);
	                
			if(iResult>0){
	                
			if((create_socket=port_exchange_client_greet(buff))){

				printf("Accepted connection from %s\nCurr port: %u\nForking...\n",inet_ntoa(state.client_udp_addr.sin_addr),curr_port);
				sock= socket(AF_INET,SOCK_DGRAM,0);
				pid=fork();

			}
			else{
				printf("Rejected connection from %s\n",inet_ntoa(state.client_udp_addr.sin_addr));
				perror("Interrupção no modulo do server!!!!\n");
				pid=1;
			}
			if(!pid){
				con_go(sock,curr_port,&state.client_udp_addr,&state.server_udp_addr);
				raise(SIGINT);
                	}
        		else if(pid<0){
                        	perror("Erro no fork no loop de conexoes do server!!!\n");
				raise(SIGINT);
			}
		}
		else if(iResult<0){

			perror("Select error:");
		}
		else{

			printf("Timed out! ( more that %ds waiting 4 udp). Trying again...Curr port: %u\n",SERVER_TIMEOUT_CON_SEC,curr_port);
               	}
	}
}
static void con_accepting_loop_single(void){

			
			int create_socket,sock;
			char buff[DEF_DATASIZE]={0};
		
			if((create_socket=port_exchange_client_greet(buff))){
				sock= socket(AF_INET,SOCK_DGRAM,0);
				printf("Accepted connection from %s\nCurr port: %u\nForking...\n",inet_ntoa(state.client_udp_addr.sin_addr),curr_port);
			}
			else{
				printf("Rejected connection from %s\n",inet_ntoa(state.client_udp_addr.sin_addr));
				perror("Interrupção no modulo do server!!!!\n");
				raise(SIGINT);
			}
			con_go(sock,curr_port,&state.client_udp_addr,&state.server_udp_addr);
			raise(SIGINT);
}

void serverInit(char* addr,uint32_t udp_s_port){


	signal(SIGINT,sigint_handler);
        signal(SIGPIPE,sigpipe_handler);
        
	logging=1;
	logstream=stderr;
	
	memset(&state,0,sizeof(server_state));

        state.server_sock_udp= socket(AF_INET,SOCK_DGRAM,0);
        int ptr=1;
        setsockopt(state.server_sock_udp,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
        if(state.server_sock_udp==-1){
                raise(SIGINT);

        }

        //especificar socket;
        /*
	fcntl(state.server_sock_udp,F_SETFL,O_ASYNC|O_NONBLOCK);
        ioctl(state.server_sock_udp,FIOASYNC,&(int){1});
        long flags= fcntl(state.server_sock_udp,F_GETFL);
        flags |= O_NONBLOCK|O_ASYNC;
        fcntl(state.server_sock_udp,F_SETFL,flags);
	*/
        
	curr_port=state.udp_s_port=udp_s_port;
	
	state.client_udp_addr.sin_family=AF_INET;
        
	state.server_udp_addr.sin_family=AF_INET;
        state.server_udp_addr.sin_port= curr_port;
        
	curr_port++;
	struct hostent* hp= gethostbyname(addr);
        memcpy(&(state.server_udp_addr.sin_addr),hp->h_addr,hp->h_length);
	
	
	printf("Endereço de receçao do server é:\n%s. Porta: %lu\n", inet_ntoa(state.server_udp_addr.sin_addr),state.udp_s_port);
	
        
	socklen_t socklength=sizeof(state.server_udp_addr);
        bind(state.server_sock_udp,(struct sockaddr*)(&state.server_udp_addr),socklength);

        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&state.server_udp_addr;
        struct in_addr ipAddr = pV4Addr->sin_addr;
        inet_ntop( AF_INET, &ipAddr, state.address_str, INET_ADDRSTRLEN );
	state.server_is_on=1;
	con_accepting_loop_single();
	printf("Server morreu!!!!! (Saida por SIGINT)\n");
	
}


/*				if(memcmp(&curr_con_addr,&state.client_udp_addr,sizeof(state.client_udp_addr))){
					
					addr_comp_err_aux("Addr comparison error:",&state.client_udp_addr,&curr_con_addr);
					raise(SIGINT);
				}
*/
