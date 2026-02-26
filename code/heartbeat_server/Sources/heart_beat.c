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
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/server_db_driving.h"
#include "../../extra_funcs/Includes/interlvl_proto.h"
#include "../Includes/heart_beat.h"


static slave_args arg_s={0};

static acceptor_args arg_a={0};

static overseer_args arg_o={0};

pthread_mutex_t	master_mtx=PTHREAD_MUTEX_INITIALIZER,
		con_mtx=PTHREAD_MUTEX_INITIALIZER,
		hb_mtx=PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t master_running_mtx=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t master_running_cond=PTHREAD_COND_INITIALIZER;

atomic_int is_on=0;
static struct sigaction sa;
static struct sigaction sa_sigpipe;

atomic_int started=0;

static con_t con_obj={0};

static void close_all_fds_here(void){


	close_all_fds(arg_o.cons);
	pthread_mutex_lock(&con_mtx);
	send_port_back(htons(arg_s.this_con_addr.sin_port),&port_mapper_ip_cache_entry);
	close_con(arg_s.con_obj,0,1);
	pthread_mutex_unlock(&con_mtx);
	pthread_cond_signal(arg_o.cons->start_cond);


}
static void call_signal_func(void){


	end_openssl_libs_server_side();
	send_port_back(htons(arg_a.accept_addr.sin_port),&port_mapper_ip_cache_entry);
	close_all_fds_here();
	perror("Saindo do heart beat server!!!!\n");
	pthread_cond_signal(&master_running_cond);
}


static void sigint_handler(int useless){

	is_on=0*useless;
	started=1;
}
static void sigpipe_handler(int useless){

	is_on+=0*useless;
}

void exit_emergency_func(void){

	//argument can be anything, really
	//sigint_handler(1);

}
void start_heart_beats(ip_cache_entry* ent_this,ip_cache_entry* ent_upper){

 	sa.sa_handler = sigint_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);

	sa_sigpipe.sa_handler = sigpipe_handler;
        sigemptyset(&sa_sigpipe.sa_mask);
        sa_sigpipe.sa_flags = SA_RESTART;
    	sigaction(SIGPIPE, &sa_sigpipe, NULL);
	exit_func_for_this_module=exit_emergency_func;

	logging=cfg_hb_server_logging;
	logstream=stdout;
	int fd_arr[MAX_SERVERS]={0};
	con_t con_arr[MAX_SERVERS]={0};

	pthread_t hb_tid_master,hb_tid_watchdog,
		master_tid;
	pthread_mutex_t hb_serv_mtx=PTHREAD_MUTEX_INITIALIZER,
			hb_cond_mtx=PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t  hb_cond=PTHREAD_COND_INITIALIZER,
			master_cond=PTHREAD_COND_INITIALIZER;

	char buff[HB_SERVER_NAME_SIZE]={0};
   	if(!strnlen(hb_server_name_buff,HB_SERVER_NAME_SIZE)){

                randStr(HB_SERVER_NAME_SIZE-1,buff);
        }
        else{

                memcpy(buff,hb_server_name_buff,min(strlen(hb_server_name_buff),sizeof(buff)-1));
	}
        char extension_buff[EXTENSION_SIZE+1]={0};
	strncpy(extension_buff,"N/A",EXTENSION_SIZE);
	con_set set={0};

	memcpy(&arg_s.slave_port_mapper_ip_cache_entry,&port_mapper_ip_cache_entry,sizeof(ip_cache_entry));
	memcpy(&arg_a.acceptor_port_mapper_ip_cache_entry,&port_mapper_ip_cache_entry,sizeof(ip_cache_entry));
	memcpy(&arg_s.slave_ip_cache_entry,ent_this,sizeof(ip_cache_entry));
	if(init_addr(&arg_s.master_addr,ent_upper->hostname,ent_upper->port)){
		perror("Erro a inicializar address de slave em heartbeat server!!!\n");
		exit(-1);
		return;
	}

	init_module_tcp_stuff(&arg_a.accept_sockfd,ent_this->hostname,ent_this->port,&arg_a.accept_addr,SIGPIPE,MAX_SERVERS,0,&arg_a.acceptor_port_mapper_ip_cache_entry);

	memcpy(&arg_s.this_addr,&arg_a.accept_addr,sizeof(struct sockaddr_in));

	arg_a.is_on=&is_on;
        arg_a.started=&started;
        arg_a.exit_signal=SIGINT;
        arg_a.clean_func=call_signal_func;
        arg_a.started=&started;
        arg_a.ack_period_us=cfg_hb_ack_period_us;



        arg_s.lower_name=buff;
        arg_s.exit_signal=SIGINT;
        arg_s.ack_period_us=cfg_hb_ack_period_us;
        arg_s.start_trigger=&started;
        arg_s.loop_var=arg_a.is_on;
        arg_s.var_mtx=&hb_mtx;

	arg_s.con_obj=&con_obj;
	arg_s.con_mtx=&con_mtx;
	arg_s.is_tls=will_use_tls;
	arg_s.sig_func=sigint_handler;
	arg_s.clean_func=call_signal_func;
	arg_s.trg_cond=&master_cond;
	arg_s.type=HB_SERVER;
	arg_s.extension_buff=extension_buff;

        arg_o.is_on=arg_a.is_on;
        arg_o.exit_signal=SIGINT;
        arg_o.sig_func=sigint_handler;
	arg_o.clean_func=call_signal_func;
        arg_o.ack_period_us=cfg_hb_ack_period_us;
        arg_o.start_cond_mtx=&hb_cond_mtx;
        arg_o.var_mtx=arg_s.var_mtx;

        arg_a.arg_o=&arg_o;
        arg_a.sig_func=sigint_handler;
	arg_a.arg_s=&arg_s;
	arg_a.ack_period_us=cfg_hb_ack_period_us;
	arg_a.is_tls=arg_s.is_tls;
	arg_a.master_mtx=&master_mtx;
        arg_a.var_mtx=arg_s.var_mtx;


	init_con_set(&set,con_arr,fd_arr,MAX_SERVERS,&hb_serv_mtx,&hb_cond);
	arg_o.cons=&set;


	memcpy(&arg_s.con_times_pair,&hb_con_times_pair,sizeof(int_pair));
	memcpy(&arg_s.data_times_pair,&hb_data_times_pair,sizeof(int_pair));
	memcpy(&arg_s.ack_times_pair,&hb_ack_times_pair,sizeof(int_pair));

	memcpy(&arg_a.con_times_pair,&hb_con_times_pair,sizeof(int_pair));
	memcpy(&arg_a.ack_times_pair,&hb_ack_times_pair,sizeof(int_pair));

	memcpy(&arg_a.data_times_pair,&hb_data_times_pair,sizeof(int_pair));


	memcpy(&arg_o.data_times_pair,&hb_data_times_pair,sizeof(int_pair));
	memcpy(&arg_o.ack_times_pair,&hb_ack_times_pair,sizeof(int_pair));

	openDB(DB_FILE);
	is_on=1;
	started=0;
	pthread_create(&master_tid,NULL,slave_thread,(void*)&arg_s);
	pthread_create(&hb_tid_master,NULL,acceptor_func,(void*)&arg_a);
	pthread_create(&hb_tid_watchdog,NULL,watch_dog_func,(void*)&arg_o);

        pthread_mutex_lock(&master_running_mtx);
        while(is_on){
                pthread_cond_wait(&master_running_cond,&master_running_mtx);
        }
        pthread_mutex_unlock(&master_running_mtx);

	pthread_join(master_tid,NULL);
	printf("Saimos do thread de replies ao master to server de heartbeat!!!!!\n");
	pthread_join(hb_tid_master,NULL);
	printf("Saimos do thread master do server de heartbeat!!!!!\n");
	pthread_join(hb_tid_watchdog,NULL);
	printf("Saimos do thread watchdog do server de heartbeat!!!!!\n");
	closeDB();
	close(arg_a.accept_sockfd);
	
}


