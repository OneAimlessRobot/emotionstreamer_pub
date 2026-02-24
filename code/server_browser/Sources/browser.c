#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/openssl_stuff.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/more_socket_ops.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/interlvl_proto.h"
#include "../Includes/browser.h"
#include "../../port_mapper/Includes/mapper.h"

static struct sockaddr_in hb_server_addr;
static struct sockaddr_in our_addr;
static int fd=1;
static con_t con_obj={0};
static struct sigaction sa;
static atomic_int innited=0;

static void cleanup_and_send_ports_back(int useless,void*ptr){

	end_openssl_libs_client_side();
	raise(useless);
	send_port_back(htons(our_addr.sin_port),&port_mapper_ip_cache_entry);
	free_attempted_ports(0,&port_mapper_ip_cache_entry);
	close_con(&con_obj,0,1);
	exit(useless+(0*((uint64_t)ptr)));

}


static void sigint_handler(int useless){


	innited=useless*0;

}


void exit_emergency_func(void){

	//argument can be anything, really
	sigint_handler(1);
	cleanup_and_send_ports_back(SIGINT,NULL);

}
static void recv_servers(void){


	clear_con_data(&con_obj);
	int result=con_read_tcp(&con_obj,browser_data_times_pair);
	if(result<0){

			if(result==-2){
				printf("timeout 1!!!\n");
			}
			else{
				perror("erro 1!!!\n");
				cleanup_and_send_ports_back(SIGINT,NULL);
			}

	}
	dprintf(fd,"%s\n",(char*)con_obj.tcp_data);
	result=con_send_tcp(&con_obj,browser_data_times_pair);
	if(result<0){

			if(result==-2){
				printf("timeout 2!!!\n");
			}
			else{
				perror("erro 2!!!\n");
				cleanup_and_send_ports_back(SIGINT,NULL);
			}

	}
	while(innited&&strs_are_strictly_equal((char*)con_obj.tcp_data,"done")){

		int result=con_read_tcp(&con_obj,browser_data_times_pair);
		if(result<0){
			if(result==-2){
				printf("timeout 3!!!\n");
				continue;
			}
			else{
				perror("erro 3!!!\n");
				break;
			}
		}
		dprintf(fd,"%s\n",(char*)con_obj.tcp_data);
		result=con_send_tcp(&con_obj,browser_data_times_pair);
		if(result<0){
			if(result==-2){
				printf("timeout 4!!!\n");
				continue;
			}
			else{
				perror("erro 4!!!\n");
				break;
			}
		}
	}
	cleanup_and_send_ports_back(SIGINT,NULL);


}


void init_browser(char* hostname, char* req,uint16_t port){
	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	logging=cfg_server_browser_logging;
	logstream=stdout;
	exit_func_for_this_module=exit_emergency_func;

        if(init_addr(&hb_server_addr,hostname,port)){

	    perror("Não conseguimos inicializar address do peer em server_browser!!!\n");
	    cleanup_and_send_ports_back(SIGINT,NULL);
	}
	init_openssl_libs_client_side();
	init_con(&con_obj,con_obj.sockfd_tcp,CLIENT_C,&port_mapper_ip_cache_entry,will_use_tls);
	connection_attempt_circuit(&con_obj.sockfd_tcp,cleanup_and_send_ports_back,&our_addr,
                                &hb_server_addr,
                                       &server_browser_ip_cache_entry,&port_mapper_ip_cache_entry,browser_con_times_pair,NULL);
        clear_con_data(&con_obj);
	char string_to_send[DEF_DATASIZE/2]={0};
	interlvl_cmd cmd= str_to_interlvl_cmd_type(req);

	switch(cmd){

		case SHOW:
			strncpy(string_to_send,SHOW_STRING,(DEF_DATASIZE/2)-1);
			break;
		case MASTER_SHOW:
			strncpy(string_to_send,SHOW_MASTER_STRING,(DEF_DATASIZE/2)-1);
			break;
		default:
			printf("Request desconhecido: |%s|\n",req);
			cleanup_and_send_ports_back(SIGINT,NULL);
        }
	greet(&con_obj,browser_con_times_pair);
	snprintf((char*)con_obj.tcp_data,DEF_DATASIZE-1,"%s",string_to_send);
        int result=con_send_tcp(&con_obj,browser_con_times_pair);
        if(result<0){


                perror("Nao deu para contactar server de heartbeats!!!! Nao recebeu pedido de login\n");
		cleanup_and_send_ports_back(SIGINT,NULL);
        }
	innited=1;
	recv_servers();


}
