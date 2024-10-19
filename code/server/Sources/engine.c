#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/connection.h"
#include "../Includes/engine.h"

static server_state state;

static void serverStop(int useless){

	state.server_is_on=0;
	exit(useless);
}
static void sigpipe_handler(int useless){

	perror("Interrupção no modulo do server!!!!\n");
	serverStop(useless);

}
static void sigint_handler(int useless){

	close(state.server_sock_tcp);
	sigpipe_handler(useless);
}

static void con_accepting_loop(void){

		while(state.server_is_on){
		
			
			int iResult,
				sock=-1,
				pid=-1;
		
	                struct timeval tv;
	                tv.tv_sec=SERVER_TIMEOUT_CON_SEC;
	                tv.tv_usec=SERVER_TIMEOUT_CON_USEC;
		

	                FD_ZERO(&state.rdfds);
	                FD_SET(state.server_sock_tcp,&state.rdfds);
			
	                iResult=select(state.server_sock_tcp+1,&state.rdfds,(fd_set*)0,(fd_set*)0,&tv);
	                
			if(iResult>0){
			
				sock= accept(state.server_sock_tcp,NULL,NULL);
				if(sock>=0){
					
					printf("Connection accepted!\n");
					setNonBlocking(sock);
					pid=fork();

				}
				else{
					perror("Rejected connection!");
					pid=1;
				}
				
			if(!pid){
				con_go(sock);
				serverStop(0);
                	}
        		else if(pid<0){
                        	perror("Erro no fork no loop de conexoes do server!!!\n");
				raise(SIGPIPE);
			}
			else{

				close(sock);
			}
		}
		else if(iResult<0){

			perror("Select error:");
		}
		else{

			printf("Timed out! ( more that %ds waiting 4 udp). Trying again...\n",SERVER_TIMEOUT_CON_SEC);
               	}
	}
}

static void setup_server_tcp_stuff(char* addr,int tcp_s_port){


	state.server_sock_tcp= socket(AF_INET,SOCK_STREAM,0);
        if(state.server_sock_tcp==-1){
                raise(SIGPIPE);

        }
	int ptr=1;
        setsockopt(state.server_sock_tcp,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
	init_addr(&state.server_tcp_addr,addr,tcp_s_port);

	bind(state.server_sock_tcp,(struct sockaddr*)(&state.server_tcp_addr),*socklenvar);

	listen(state.server_sock_tcp,MAX_CLIENTS_HARD_LIMIT);
}

void serverInit(char* addr,uint32_t tcp_s_port){


	signal(SIGINT,sigint_handler);
        signal(SIGPIPE,sigpipe_handler);
	logging=1;
	logstream=stderr;
	memset(&state,0,sizeof(server_state));
	setup_server_tcp_stuff(addr,tcp_s_port);
	state.server_is_on=1;
	con_accepting_loop();
	
}

