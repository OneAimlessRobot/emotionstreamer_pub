#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/interlvl_proto.h"
#include "../Includes/browser.h"

static int tcp_sock=-1;
static struct sockaddr_in hb_server_addr;
static struct sockaddr_in our_addr;
static int fd=1;
static con_t con_obj={0};

static void sigint_handler(int useless){

	perror("Saimos do server browser!!!!\n");
	close_con(&con_obj);
	exit(useless);
	
}
static void sigpipe_handler(int useless){


	sigint_handler(useless);

}


static void recv_servers(void){


	clear_con_data(&con_obj);
	int result=con_read_udp(&con_obj,browser_data_times_pair);
	if(result<0){

			if(result==-2){
				printf("timeout 1!!!\n");
			}
			else{
				perror("erro 1!!!\n");
				raise(SIGINT);
			}

	}
	dprintf(fd,"%s\n",(char*)con_obj.udp_data);
	result=con_send_udp(&con_obj,browser_data_times_pair);
	if(result<0){

			if(result==-2){
				printf("timeout 2!!!\n");
			}
			else{
				perror("erro 2!!!\n");
				raise(SIGINT);
			}

	}
	
	while(strs_are_strictly_equal((char*)con_obj.udp_data,"done")){

		int result=con_read_udp(&con_obj,browser_data_times_pair);
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
		
		dprintf(fd,"%s\n",(char*)con_obj.udp_data);
			
		result=con_send_udp(&con_obj,browser_data_times_pair);
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
	raise(SIGINT);



}


void init_browser(char* hostname, char* req,uint16_t port){
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigpipe_handler);


        con_obj.sockfd_tcp=tcp_sock= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(con_obj.sockfd_tcp<0){
                perror("Socket nao criada no hb thread!!!\n");
                raise(SIGINT);
        }


        init_addr(&hb_server_addr,hostname,port);

        print_addr_aux("Addr atual do server de heartbeat:",&hb_server_addr);

        if(!tryConnect(&tcp_sock,browser_con_times_pair,&hb_server_addr)){

                perror("Nao deu para contactar server de heartbeats!!!!\n");
                raise(SIGINT);
        }

        init_con(&con_obj,con_obj.sockfd_tcp,CLIENT_C);


        getsockname(tcp_sock,(struct sockaddr*)&our_addr,socklenvar);

        greet(&con_obj,browser_con_times_pair,our_addr.sin_port);

        clear_con_data(&con_obj);
	char string_to_send[PATHSIZE/2]={0};
        
	interlvl_cmd cmd= str_to_interlvl_cmd_type(req);
	
	switch(cmd){
	
		case SHOW:
			strncpy(string_to_send,SHOW_STRING,(PATHSIZE/2)-1);
			break;
		case MASTER_SHOW:
			strncpy(string_to_send,SHOW_MASTER_STRING,(PATHSIZE/2)-1);
			break;
		default:
			printf("Request desconhecido: |%s|\n",req);
			return;
			
	}

	snprintf((char*)con_obj.ack_udp_data,DEF_DATASIZE-1,"%s",string_to_send);
        int result=con_send_udp_ack(&con_obj,browser_con_times_pair);
	
        if(result<0){


                perror("Nao deu para contactar server de heartbeats!!!! Nao recebeu pedido de login\n");
                raise(SIGINT);

        }

	recv_servers();


}
