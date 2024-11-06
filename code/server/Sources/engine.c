#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/interlvl_com.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/engine.h"
#include "../Includes/connection.h"


static server_state state;
static uint16_t curr_port=0;
static pthread_t hb_tid;
static pthread_mutex_t hb_mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t eng_mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t eng_cond=PTHREAD_COND_INITIALIZER;
static int started=0;
static void serverStop(int useless){
	
	perror("Sinal de parar server\n");
	close(state.server_sock_tcp);
	acess_var_mtx(&hb_mtx,&state.server_is_on,0*useless,V_SET);
	acess_var_mtx(&eng_mtx,&started,1,V_SET);
	pthread_cond_signal(&eng_cond);

}
static void conStop(int useless){
	
	perror("Sinal de parar con\n");
	close(state.server_sock_tcp);
	acess_var_mtx(&hb_mtx,&state.server_is_on,0*useless,V_SET);

}

static int con_accepting_loop(void){

		printf("Chegamos ao loop de conexoes!\n");
		pthread_mutex_lock(&eng_mtx);
		while(!acess_var_mtx(&hb_mtx,&started,0,V_LOOK)){
			printf("Esperando sinal do hearbeat thread!!!\n");
			pthread_cond_wait(&eng_cond,&eng_mtx);
		}
		pthread_mutex_unlock(&eng_mtx);


		while(acess_var_mtx(&hb_mtx,&state.server_is_on,0,V_LOOK)){

			int iResult,
				pid=-1,
				sock=-1;
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
					curr_port+=4;
					pid=1;
					pid=fork();
					switch(pid){
						case 0:
							setNonBlocking(sock);
							signal(SIGTERM,conStop);
							signal(SIGINT,conStop);
							signal(SIGPIPE,conStop);
							raise(SIGINT);
							close_con(&state.hb_con);
							con_go(sock,curr_port-4);
							return 0;
						case -1:
							raise(SIGTERM);
							return 1;
						default:
							close(sock);

							break;
					}
				}
				else{
					perror("Rejected connection!");
					
				}
			}
		else if(iResult<0){

			perror("Select error:");
			return 1;
		}
		else{
			
			printf("Timed out! ( more that %lus waiting 4 udp). Trying again...\n",server_con_times_pair[0]);
			printf("Nome do server atual: %s\n",state.name);
			print_addr_aux("Endereço de server atual:",&state.server_tcp_addr);
               		
		}
	}
	printf("Fechou a loja!!!\n");
	return 1;

}

int serverInit(ip_cache_entry* ent_this,ip_cache_entry* ent_upper){

	signal(SIGTERM,serverStop);
	signal(SIGINT,serverStop);
	signal(SIGPIPE,serverStop);
	char buff[SERVER_NAME_SIZE]={0};
	randStr(SERVER_NAME_SIZE-1,buff);
	curr_port=ent_this->port;
	logging=1;
	logstream=stderr;
	memset(&state,0,sizeof(server_state));
	state.name=buff;
	init_module_tcp_stuff(&state.server_sock_tcp,ent_this->hostname,ent_this->port,&state.server_tcp_addr,SIGTERM,MAX_CLIENTS_HARD_LIMIT);
	
	slave_args arg_s={0};
	

	arg_s.send_str=HB_SEND_STRING;
	arg_s.reply_str=HB_REPLY_STRING;
	arg_s.lower_name=buff;
	arg_s.exit_signal=SIGTERM;
	arg_s.ack_timeout_lim= server_ack_timeout_lim;
	arg_s.sleep_us=10000;
	arg_s.start_trigger=&started;
	arg_s.loop_var=&state.server_is_on;
	arg_s.var_mtx=&hb_mtx;
	arg_s.trg_cond=&eng_cond;
	arg_s.type=SERVER;
	
	init_addr(&arg_s.master_addr,ent_upper->hostname,ent_upper->port);
	init_addr(&arg_s.this_addr,ent_this->hostname,ent_this->port);
	memcpy(&arg_s.con_times_pair,&server_con_times_pair,sizeof(int_pair));
	memcpy(&arg_s.data_times_pair,&server_data_times_pair,sizeof(int_pair));


	state.server_is_on=1;
	started=0;
	
	pthread_create(&hb_tid,NULL,slave_thread,(void*)&arg_s);
	int result=con_accepting_loop();
	if(result){
	pthread_join(hb_tid,NULL);
	printf("Juntamos o thread hb_tid\n");
	close_con(&state.hb_con);
	}
	return result;
}

