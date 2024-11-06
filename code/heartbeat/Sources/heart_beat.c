#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/interlvl_com.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/server_db_driving.h"
#include "../../extra_funcs/Includes/interlvl_proto.h"
#include "../Includes/heart_beat.h"


static slave_args arg_s={0};

static overseer_args arg_o={0};

static hb_acceptor acceptor={0};

pthread_mutex_t	master_mtx=PTHREAD_MUTEX_INITIALIZER;

static int curr_port=0;

static int started=0;

static void close_all_fds_here(void){



	close_all_fds(arg_o.cons);
	close(acceptor.accept_sockfd);
	pthread_cond_signal(arg_o.cons->start_cond);


}

static void sigint_handler(int useless){

	acess_var_mtx(arg_s.var_mtx,&acceptor.is_on,0*useless,V_SET);
	close_all_fds_here();
	perror("Saindo do heart beat server!!!!\n");

}

static void sigpipe_handler(int useless){

	sigint_handler(useless);

}

void* heart_beat_func(void* args){

 	printf("Chegamos ao loop de heart beat!\n");
        pthread_mutex_lock(&master_mtx);
        while(!acess_var_mtx(arg_s.var_mtx,&started,0,V_LOOK)){
                 printf("Esperando sinal do master server thread!!!\n");
                 pthread_cond_wait(arg_s.trg_cond,&master_mtx);
        }
        pthread_mutex_unlock(&master_mtx);
	curr_port++;


        char req_buff[PATHSIZE/4]={0};
	char ip_buff[PATHSIZE/4]={0};
	char name_buff[PATHSIZE/4]={0};
	char type_buff[PATHSIZE/4]={0};
	char big_buff[PATHSIZE*6]={0};
	int result=0;
	int iResult,
	       sock=-1;
	while(acess_var_mtx(arg_s.var_mtx,&acceptor.is_on,0,V_LOOK)){


		              
		memset(big_buff,0,sizeof(big_buff));
		struct timeval tv;
		tv.tv_sec=hb_con_times_pair[0];
		tv.tv_usec=hb_con_times_pair[1];
		FD_ZERO(&acceptor.mainfds);
		FD_SET(acceptor.accept_sockfd,&acceptor.mainfds);
		iResult=select(acceptor.accept_sockfd+1,&acceptor.mainfds,(fd_set*)0,(fd_set*)0,&tv);
		if(iResult>0){
			
			con_t con={0};
			sock= accept(acceptor.accept_sockfd,NULL,NULL);
                        
			if(sock>=0){
			      
			      printf("Connection accepted!\n");
                              setNonBlocking(sock);
			      init_con(&con,sock,SERVER_C);
			      uint16_t stored_port=0;
			      greet(&con,hb_con_times_pair,curr_port);
			      clear_con_data(&con);
			      result=con_read_udp_ack(&con,hb_data_times_pair);
			      sscanf((char*)con.ack_udp_data,"%s %s %s %s %hu ",req_buff,type_buff,name_buff,ip_buff,&stored_port);
			      if(result<=0){
					perror("Nao sabemos o que querem....\n");
					close_con(&con);
					continue;
			      }
			   
			      result=con_send_udp_ack(&con,hb_data_times_pair);
			      if(result<=0){
					perror("Nao sabemos o que querem....\n");
					close_con(&con);
					continue;
			      }

			      interlvl_cmd cmd=str_to_interlvl_cmd_type((char*)req_buff);
                              clear_con_data(&con);
                              switch(cmd){


                                case MASTER_SHOW:
                                        snprint_addr_aux(ip_buff,PATHSIZE/4,&arg_s.master_addr);
                                        snprintf((char*)con.udp_data,DEF_DATASIZE-1,"Nao sou um master."
                                                                                     "Mas, se quiseres, Está aqui o meu master."
                                                                                     "Tenta falar com ele: %s:%hu\n",
                                                                                                ip_buff,ntohs(arg_s.master_addr.sin_port));
                                        result=con_send_udp(&con,hb_data_times_pair);
                                        clear_con_data(&con);
                                        snprintf((char*)con.udp_data,DEF_DATASIZE-1,"done");
                                        result=con_send_udp(&con,hb_data_times_pair);
                                        close_con(&con);
                                        break;

                                case SHOW:
                                        printf("Show servers requested!!!!\n");
                                        show_servers(&con,hb_data_times_pair);
                                        close_con(&con);
                                        break;
                                case LOG:
                                        printf("Log server requested!!!!\n");
                                        add_con(arg_o.cons,&con,type_buff,sock,name_buff,ip_buff,stored_port);
                                        curr_port+=3;
                                        break;
                                default:
                                        printf("Request desconhecido %s!!!!\n",req_buff);
                                        close_con(&con);
                                        break;
		
			      }

			}
                        else{
                             perror("Rejected connection!");
                        }

		}
		else if(iResult<0){
			perror("Erro no select no thread de heartbeats!!!!\n");
			raise(SIGINT);
		}
		print_addr_aux("Nada....\n",&arg_s.this_addr);


	}
	printf("Saimos do thread de heart_beat_master!!!!\n");



	return args;



}
void start_heart_beats(ip_cache_entry* ent_this,ip_cache_entry* ent_upper){


	int fd_arr[MAX_SERVERS]={0},
		timeout_arr[MAX_SERVERS]={0};
	con_t con_arr[MAX_SERVERS]={0};

	pthread_t hb_tid_master,hb_tid_watchdog,
		master_tid;
	pthread_mutex_t hb_mtx=PTHREAD_MUTEX_INITIALIZER,
			hb_serv_mtx=PTHREAD_MUTEX_INITIALIZER,
			hb_cond_mtx=PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t  hb_cond=PTHREAD_COND_INITIALIZER,
			master_cond=PTHREAD_COND_INITIALIZER;

	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	openDB(DB_FILE);
	char buff[HB_SERVER_NAME_SIZE]={0};
        randStr(HB_SERVER_NAME_SIZE-1,buff);
	curr_port=ent_this->port;

	con_set set={0};

	init_addr(&arg_s.master_addr,ent_upper->hostname,ent_upper->port);
	init_addr(&arg_s.this_addr,ent_this->hostname,ent_this->port);


	init_module_tcp_stuff(&acceptor.accept_sockfd,ent_this->hostname,ent_this->port,&arg_s.this_addr,SIGPIPE,MAX_SERVERS);
	acceptor.is_on=1;


        arg_s.lower_name=buff;
        arg_s.exit_signal=SIGINT;
        arg_s.ack_timeout_lim= hb_ack_timeout_lim;
        arg_s.sleep_us=100000;
        arg_s.start_trigger=&started;
        arg_s.loop_var=&acceptor.is_on;
        arg_s.var_mtx=&hb_mtx;
        arg_s.trg_cond=&master_cond;
	arg_s.type=HB_SERVER;

        arg_o.is_on=&acceptor.is_on;
        arg_o.exit_signal=SIGINT;
        arg_o.ack_timeout_lim= hb_ack_timeout_lim;
        arg_o.start_cond_mtx=&hb_cond_mtx;
        arg_o.var_mtx=arg_s.var_mtx;


	init_con_set(&set,con_arr,timeout_arr,fd_arr,MAX_SERVERS,&hb_serv_mtx,&hb_cond);
	arg_o.cons=&set;


	memcpy(&arg_s.con_times_pair,&hb_con_times_pair,sizeof(int_pair));
	memcpy(&arg_s.data_times_pair,&hb_data_times_pair,sizeof(int_pair));

	memcpy(&arg_o.data_times_pair,&hb_data_times_pair,sizeof(int_pair));

	pthread_create(&master_tid,NULL,slave_thread,(void*)&arg_s);
	pthread_create(&hb_tid_master,NULL,heart_beat_func,NULL);
	pthread_create(&hb_tid_watchdog,NULL,watch_dog_func,(void*)&arg_o);

	pthread_join(master_tid,NULL);
	printf("Saimos do thread de replies ao master to server de heartbeat!!!!!\n");
	pthread_join(hb_tid_master,NULL);
	printf("Saimos do thread master do server de heartbeat!!!!!\n");
	pthread_join(hb_tid_watchdog,NULL);
	printf("Saimos do thread watchdog do server de heartbeat!!!!!\n");

	closeDB();
}


