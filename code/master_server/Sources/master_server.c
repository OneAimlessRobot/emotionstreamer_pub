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




static overseer_args arg_o={0};


static master_acceptor acceptor={0};

static int curr_port=0;

pthread_mutex_t master_mtx=PTHREAD_MUTEX_INITIALIZER;

static void close_all_fds_here(void){



        close_all_fds(arg_o.cons);
        close(acceptor.accept_sockfd);
        pthread_cond_signal(arg_o.cons->start_cond);


}


static void sigint_handler(int useless){

	acess_var_mtx(arg_o.var_mtx,&acceptor.is_on,0*useless,V_SET);
	close_all_fds_here();
	perror("Saindo do heart beat server!!!!\n");

}

static void sigpipe_handler(int useless){

	sigint_handler(useless);

}



void* master_server_func(void* args){

        char req_buff[PATHSIZE/4]={0};
	char ip_buff[PATHSIZE/4]={0};
	char name_buff[PATHSIZE/4]={0};
	char type_buff[PATHSIZE/4]={0};
	char big_buff[PATHSIZE*6]={0};
	int result=0;
	int iResult,
	       sock=-1;
	while(acess_var_mtx(&master_mtx,&acceptor.is_on,0,V_LOOK)){


		              
		memset(big_buff,0,sizeof(big_buff));
		struct timeval tv;
		tv.tv_sec=master_con_times_pair[0];
		tv.tv_usec=master_con_times_pair[1];
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
			      greet(&con,master_con_times_pair,curr_port);
			      clear_con_data(&con);
			      result=con_read_udp_ack(&con,master_data_times_pair);
			      sscanf((char*)con.ack_udp_data,"%s %s %s %s %hu ",req_buff,type_buff,name_buff,ip_buff,&stored_port);
			      if(result<=0){
					perror("Nao sabemos o que querem....\n");
					close_con(&con);
					continue;
			      }
			      result=con_send_udp_ack(&con,master_data_times_pair);
			      if(result<=0){
					perror("Nao sabemos o que querem....\n");
					close_con(&con);
					continue;
			      }
			      interlvl_cmd cmd=str_to_interlvl_cmd_type((char*)req_buff);
			      clear_con_data(&con);
			      switch(cmd){


				case MASTER_SHOW:
                                        snprint_addr_aux(ip_buff,PATHSIZE/4,&acceptor.tcp_addr);
                                        snprintf((char*)con.udp_data,DEF_DATASIZE-1,"Nao sou um master."
                                                                                     "Mas, se quiseres, Está aqui o meu master."
                                                                                     "Tenta falar com ele: %s:%hu\n",
                                                                                                ip_buff,ntohs(acceptor.tcp_addr.sin_port));
                                        result=con_send_udp(&con,master_data_times_pair);
                                        clear_con_data(&con);
                                        snprintf((char*)con.udp_data,DEF_DATASIZE-1,"done");
                                        result=con_send_udp(&con,master_data_times_pair);
                                        close_con(&con);
					break;

				case SHOW:
					printf("Show servers requested!!!!\n");
					show_servers(&con,master_data_times_pair);
					close_con(&con);
					break;
				case LOG:
		 	                printf("Log server requested!!!!\n");
					memset(big_buff,0,PATHSIZE*3);
					snprintf(big_buff,sizeof(big_buff)-1,"%s, %d, %s, '%s:%hu'",type_buff,sock,name_buff,ip_buff,htons(stored_port));
					add_con(arg_o.cons,&con,big_buff);
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
		print_addr_aux("Nada....\n",&acceptor.tcp_addr);


	}
	printf("Saimos do thread de heart_beat_master!!!!\n");



	return args;



}
static void setup_server_tcp_stuff(char* addr,uint16_t tcp_s_port){


        acceptor.accept_sockfd= socket(AF_INET,SOCK_STREAM,0);
        if(acceptor.accept_sockfd==-1){
                perror("Nao foi possivel criar a socket principal no heart beat server!!!!\n");
                raise(SIGPIPE);

        }
        int ptr=1;
        setsockopt(acceptor.accept_sockfd,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
        init_addr(&acceptor.tcp_addr,addr,tcp_s_port);

        if(bind(acceptor.accept_sockfd,(struct sockaddr*)(&acceptor.tcp_addr),*socklenvar)){
                perror("Nao foi possivel fazer bind da socket principal no heart beat server!!!!\n");
		raise(SIGPIPE);

        }

        listen(acceptor.accept_sockfd,MAX_HB_SERVERS);
	printf("Listening!!!!!\n");
}



void start_master(char* hostname, uint16_t port){

	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);
	int fd_arr[MAX_HB_SERVERS]={0},
		timeout_arr[MAX_HB_SERVERS]={0};

	con_t con_arr[MAX_HB_SERVERS]={0};

	con_set set={0};

	pthread_t master_tid_master,master_tid_watchdog;
	pthread_mutex_t master_serv_mtx=PTHREAD_MUTEX_INITIALIZER,
			master_cond_mtx=PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t master_cond=PTHREAD_COND_INITIALIZER;


	curr_port=port;

	openDB(DB_FILE);

        init_module_tcp_stuff(&acceptor.accept_sockfd,hostname,port,&acceptor.tcp_addr,SIGPIPE,MAX_HB_SERVERS);

	acceptor.is_on=1;


        arg_o.is_on=&acceptor.is_on;
        arg_o.exit_signal=SIGINT;
        arg_o.ack_timeout_lim= master_ack_timeout_lim;
        arg_o.start_cond_mtx=&master_cond_mtx;
        arg_o.var_mtx=&master_mtx;



        init_con_set(&set,con_arr,timeout_arr,fd_arr,MAX_HB_SERVERS,&master_serv_mtx,&master_cond);

	arg_o.cons=&set;

	memcpy(&arg_o.data_times_pair,&master_data_times_pair,sizeof(int_pair));


	pthread_create(&master_tid_master,NULL,master_server_func,NULL);

	pthread_create(&master_tid_watchdog,NULL,watch_dog_func,(void*)&arg_o);


	pthread_join(master_tid_master,NULL);
	printf("Saimos do thread principal do master server!!!!\n");
	pthread_join(master_tid_watchdog,NULL);
	printf("Saimos do thread watchdog do master server!!!!\n");

	closeDB();
}


