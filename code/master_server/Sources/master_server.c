#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/openssl_stuff.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/interlvl_com.h"
#include "../../extra_funcs/Includes/server_db_driving.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/interlvl_proto.h"
#include "../Includes/master_server.h"



static acceptor_args arg_a={0};

static overseer_args arg_o={0};

pthread_mutex_t master_mtx=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t master_running_mtx=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t master_con_mtx=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t master_running_cond=PTHREAD_COND_INITIALIZER;


atomic_int is_on = 0;

static struct sigaction sa;
static struct sigaction sa_sigpipe;

atomic_int started= 0;

static void close_all_fds_here(void){


        close_all_fds(arg_o.cons);
        pthread_cond_broadcast(arg_o.cons->start_cond);
}

static void call_signal_func(void){


	end_openssl_libs_server_side();
	close_all_fds_here();
        pthread_mutex_lock(&master_con_mtx);
	send_port_back(htons(arg_a.accept_addr.sin_port),&port_mapper_ip_cache_entry);
	pthread_cond_signal(&master_running_cond);
	perror("Saindo do heart beat server!!!!\n");


}

static void sigint_handler(int useless){

	is_on=0*useless;
	started=1;
}
static void sigpipe_handler(int useless){

	is_on+=0*useless;
}


void start_master(char* hostname, uint16_t port){

	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);

	sa_sigpipe.sa_handler = sigpipe_handler;
	sigemptyset(&sa_sigpipe.sa_mask);
	sa_sigpipe.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sa_sigpipe, NULL);

	logging=cfg_master_server_logging;
	logstream=stdout;

	int fd_arr[MAX_HB_SERVERS]={0};
	con_t con_arr[MAX_HB_SERVERS]={0};

	pthread_t master_tid_master,master_tid_watchdog;
	pthread_mutex_t master_serv_mtx=PTHREAD_MUTEX_INITIALIZER,
			master_cond_mtx=PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t master_cond=PTHREAD_COND_INITIALIZER;


        con_set set={0};



	memcpy(&arg_a.acceptor_port_mapper_ip_cache_entry,&port_mapper_ip_cache_entry,sizeof(ip_cache_entry));

        init_module_tcp_stuff(&arg_a.accept_sockfd,hostname,port,&arg_a.accept_addr,SIGPIPE,MAX_HB_SERVERS,0,&arg_a.acceptor_port_mapper_ip_cache_entry);
	is_on=1;
        arg_a.is_on=&is_on;
	arg_a.sig_func=sigint_handler;
	arg_a.clean_func=call_signal_func;
        arg_a.started=&started;
	arg_a.ack_period_us=cfg_master_ack_period_us;
        arg_a.exit_signal=SIGINT;

	arg_o.is_on=&is_on;
        arg_o.exit_signal=SIGINT;
	arg_o.sig_func=sigint_handler;
        arg_o.clean_func=call_signal_func;
	arg_o.ack_period_us=cfg_master_ack_period_us;
        arg_o.start_cond_mtx=&master_cond_mtx;
        arg_o.var_mtx=&master_mtx;


        arg_a.arg_o=&arg_o;
        arg_a.arg_s=NULL;
        arg_a.con_mtx=&master_con_mtx;
        arg_a.is_tls=will_use_tls;
	arg_a.var_mtx=&master_mtx;
	arg_a.ack_period_us=cfg_master_ack_period_us;
	arg_a.master_mtx=arg_o.start_cond_mtx;

        init_con_set(&set,con_arr,fd_arr,MAX_HB_SERVERS,&master_serv_mtx,&master_cond);

        arg_o.cons=&set;


        memcpy(&arg_a.con_times_pair,&master_con_times_pair,sizeof(int_pair));
        memcpy(&arg_a.data_times_pair,&master_data_times_pair,sizeof(int_pair));
        memcpy(&arg_a.ack_times_pair,&master_ack_times_pair,sizeof(int_pair));

        memcpy(&arg_o.data_times_pair,&master_data_times_pair,sizeof(int_pair));
        memcpy(&arg_o.ack_times_pair,&master_ack_times_pair,sizeof(int_pair));


	openDB(DB_FILE);


	pthread_create(&master_tid_master,NULL,acceptor_func,(void*)&arg_a);

	pthread_create(&master_tid_watchdog,NULL,watch_dog_func,(void*)&arg_o);

	pthread_mutex_lock(&master_running_mtx);
	while(is_on){
		pthread_cond_wait(&master_running_cond,&master_running_mtx);
	}
	pthread_mutex_unlock(&master_running_mtx);

	pthread_join(master_tid_master,NULL);
	printf("Saimos do thread principal do master server!!!!\n");
	pthread_join(master_tid_watchdog,NULL);
	printf("Saimos do thread watchdog do master server!!!!\n");
	closeDB();
	close(arg_a.accept_sockfd);
}


