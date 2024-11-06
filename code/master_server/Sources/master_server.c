#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/interlvl_com.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/server_db_driving.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/interlvl_proto.h"
#include "../Includes/master_server.h"



static acceptor_args arg_a={0};

static overseer_args arg_o={0};

pthread_mutex_t master_mtx=PTHREAD_MUTEX_INITIALIZER;

static int is_on=0;

static int started=0;


static void close_all_fds_here(void){


        close_all_fds(arg_o.cons);
        pthread_cond_signal(arg_o.cons->start_cond);
}

static void sigint_handler(int useless){

	closeDB();
        close(arg_a.accept_sockfd);
	if(acess_var_mtx(&master_mtx,&is_on,0,V_LOOK)){
                acess_var_mtx(&master_mtx,&is_on,0*useless,V_SET);
                close_all_fds_here();
                perror("Saindo do heart beat server!!!!\n");
        }
        else{
                exit(useless);
        }
}

static void sigpipe_handler(int useless){

        close(arg_a.accept_sockfd);
	if(acess_var_mtx(&master_mtx,&is_on,0,V_LOOK)){
                sigint_handler(useless);
        }
        else{
                exit(useless);
        }
}

void start_master(char* hostname, uint16_t port){

	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	int fd_arr[MAX_HB_SERVERS]={0},
		timeout_arr[MAX_HB_SERVERS]={0};

	con_t con_arr[MAX_HB_SERVERS]={0};

	pthread_t master_tid_master,master_tid_watchdog;
	pthread_mutex_t master_serv_mtx=PTHREAD_MUTEX_INITIALIZER,
			master_cond_mtx=PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t master_cond=PTHREAD_COND_INITIALIZER;


        con_set set={0};

        init_addr(&arg_a.accept_addr,hostname,port);


        init_module_tcp_stuff(&arg_a.accept_sockfd,hostname,port,&arg_a.accept_addr,SIGPIPE,MAX_HB_SERVERS);

        is_on=1;
        arg_a.is_on=&is_on;
        arg_a.started=&started;
        arg_a.exit_signal=SIGINT;

	arg_o.is_on=&is_on;
        arg_o.exit_signal=SIGINT;
	arg_o.ack_timeout_lim= master_ack_timeout_lim;
        arg_o.start_cond_mtx=&master_cond_mtx;
        arg_o.var_mtx=&master_mtx;


        arg_a.arg_o=&arg_o;
        arg_a.arg_s=NULL;
        arg_a.var_mtx=&master_mtx;
	arg_a.master_mtx=arg_o.start_cond_mtx;

        init_con_set(&set,con_arr,timeout_arr,fd_arr,MAX_HB_SERVERS,&master_serv_mtx,&master_cond);

        arg_o.cons=&set;


        memcpy(&arg_a.con_times_pair,&master_con_times_pair,sizeof(int_pair));
        memcpy(&arg_a.data_times_pair,&master_data_times_pair,sizeof(int_pair));

        memcpy(&arg_o.data_times_pair,&master_data_times_pair,sizeof(int_pair));


	openDB(DB_FILE);


	pthread_create(&master_tid_master,NULL,acceptor_func,(void*)&arg_a);

	pthread_create(&master_tid_watchdog,NULL,watch_dog_func,(void*)&arg_o);


	pthread_join(master_tid_master,NULL);
	printf("Saimos do thread principal do master server!!!!\n");
	pthread_join(master_tid_watchdog,NULL);
	printf("Saimos do thread watchdog do master server!!!!\n");

	closeDB();
}


