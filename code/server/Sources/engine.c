#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/load_html.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/interlvl_com.h"
#include "../Includes/engine.h"
#include "../Includes/connection.h"
#include <sys/wait.h>


static server_state state;
static uint16_t curr_port=0;
static pthread_t hb_tid;
static pthread_mutex_t hb_mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t con_mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t eng_mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t eng_cond=PTHREAD_COND_INITIALIZER;
atomic_int started=0;
atomic_int is_on=0;
static struct sigaction sa;
static struct sigaction sa_chld;
int child_pid=-1;

static void call_sigint(void){

	close(state.server_sock_tcp);
	perror("Sinal de parar server\n");
	pthread_mutex_lock(&con_mtx);
	send_port_back(htons(state.server_tcp_addr.sin_port),&server_port_mapper_ip_cache_entry);
	send_ports_back(&state.hb_con);
	close_con(&state.hb_con);
	if(state.hb_con.sockfd_tcp>=0){
		close(state.hb_con.sockfd_tcp);
		state.hb_con.sockfd_tcp=-1;
	}
	pthread_mutex_unlock(&con_mtx);

}
static void call_sigint_sub_connection(void){

	close(state.server_sock_tcp);
	perror("Sinal de parar sub conexão em server server\n");
	pthread_mutex_lock(&con_mtx);
	close_con(&state.hb_con);
	pthread_mutex_unlock(&con_mtx);

}
static void call_sigint_chld(int useless){

	is_on+=0*useless;
	started+=0*useless;
}
static void serverStop(int useless){

	is_on=0*useless;
	started=1;
}
static void conStop(int useless){

	is_on=0*useless;
	started=1;
}

static int con_accepting_loop(void){

		printf("Chegamos ao loop de conexoes!\n");
		pthread_mutex_lock(&eng_mtx);
		while(!started){
			printf("Esperando sinal do hearbeat thread!!!\n");
			pthread_cond_wait(&eng_cond,&eng_mtx);
		}
		pthread_mutex_unlock(&eng_mtx);


		while(is_on){

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
					pid=1;
					child_pid=pid=fork();
					switch(pid){
						case 0:
							setNonBlocking(sock);
						        sa.sa_handler = conStop;
						        sigemptyset(&sa.sa_mask);
						        sa.sa_flags = SA_RESTART;
						        sigaction(SIGINT, &sa, NULL);
						        sigaction(SIGPIPE, &sa, NULL);
						        sigaction(SIGTERM, &sa, NULL);

							con_go(sock,curr_port);
							call_sigint_sub_connection();
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

        sa.sa_handler = serverStop;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGPIPE, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

        sa_chld.sa_handler = call_sigint_chld;
        sigemptyset(&sa_chld.sa_mask);
        sa_chld.sa_flags = SA_RESTART|SA_NOCLDWAIT;
	sigaction(SIGCHLD, &sa_chld, NULL);

	char buff[SERVER_NAME_SIZE]={0};
	char extension_buff[EXTENSION_SIZE+1]={0};
	strncpy(extension_buff,server_working_extension,EXTENSION_SIZE+1);
	if(!strnlen(server_name_buff,SERVER_NAME_SIZE)){

		randStr(SERVER_NAME_SIZE-1,buff);
	}
	else{

		memcpy(buff,server_name_buff,min(strlen(server_name_buff),sizeof(buff)-1));
	}
	is_wav_mode=(int8_t)(!strs_are_strictly_equal(extension_buff,WAV_MODE_EXTENSION));
	if(is_wav_mode){
		printf("Launched in '.wav' mode!!!\n");
	}

	slave_args arg_s={0};
	curr_port=ent_this->port;
	logging=1;
	logstream=stderr;
	memset(&state,0,sizeof(server_state));
	state.name=buff;
	memcpy(&arg_s.slave_port_mapper_ip_cache_entry,&server_port_mapper_ip_cache_entry,sizeof(ip_cache_entry));
	memcpy(&arg_s.slave_ip_cache_entry,ent_this,sizeof(ip_cache_entry));
	init_module_tcp_stuff(&state.server_sock_tcp,ent_this->hostname,ent_this->port,&state.server_tcp_addr,SIGTERM,MAX_CLIENTS_HARD_LIMIT,0,&arg_s.slave_port_mapper_ip_cache_entry);


	arg_s.lower_name=buff;
	arg_s.exit_signal=SIGTERM;
	arg_s.ack_timeout_lim= server_ack_timeout_lim;
	arg_s.sleep_us=10000;
	arg_s.con_obj=&state.hb_con;
	arg_s.clean_func=call_sigint;
	arg_s.ack_period_us=cfg_server_ack_period_us;
	arg_s.sig_func=serverStop;
	arg_s.start_trigger=&started;
	arg_s.loop_var=&is_on;
	arg_s.var_mtx=&hb_mtx;
	arg_s.con_mtx=&con_mtx;
	arg_s.trg_cond=&eng_cond;
	arg_s.type=SERVER;
	arg_s.extension_buff=extension_buff;
	if(init_addr(&arg_s.master_addr,ent_upper->hostname,ent_upper->port)){
		perror("Erro a inicializar address de master em server!!!\n");
                raise(SIGINT);
                call_sigint();
		return 1;
	}
	if(init_addr(&arg_s.this_addr,ent_this->hostname, ntohs(state.server_tcp_addr.sin_port))){

		perror("Erro a inicializar address de slave em server!!!\n");
                raise(SIGINT);
                call_sigint();
		return 1;
	}
	memcpy(&arg_s.con_times_pair,&server_con_times_pair,sizeof(int_pair));
	memcpy(&arg_s.data_times_pair,&server_data_times_pair,sizeof(int_pair));
	memcpy(&arg_s.holepunching_times_pair,&server_holepunching_times_pair,sizeof(int_pair));
	memcpy(&arg_s.ack_times_pair,&server_ack_times_pair,sizeof(int_pair));
	

	is_on=1;
	started=0;
	
	pthread_create(&hb_tid,NULL,slave_thread,(void*)&arg_s);
	int result=con_accepting_loop();
	if(result){
	pthread_join(hb_tid,NULL);
	printf("Juntamos o thread hb_tid\n");
	call_sigint();
	close_con(&state.hb_con);
	}
	return result;
}

