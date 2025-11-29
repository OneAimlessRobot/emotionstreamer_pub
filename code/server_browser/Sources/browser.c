#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/more_socket_ops.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/interlvl_proto.h"
#include "../Includes/browser.h"
#include "../../port_mapper/Includes/mapper.h"
static port_array attempted_port_arr={0};

static int num_attempted_ports=0;

static char string_to_send[DEF_DATASIZE]={0};
static struct sockaddr_in hb_server_addr;
static struct sockaddr_in our_addr;
static int forceful_teardown=0;
static int fd=1;
static con_t con_obj={0};
static struct sigaction sa;
static atomic_int innited=0;
static void free_attempted_ports(int success){

	memset(string_to_send,0,sizeof(string_to_send));
	char* ptr=string_to_send;
        for(int i=0;i<(num_attempted_ports)-(success!=0);i++){
                if(!i){
                        ptr+=snprintf(ptr,sizeof(string_to_send)-(ptr-string_to_send),"%d ",num_attempted_ports-(success!=0));
                }
                if(attempted_port_arr[i]){
                        ptr+=snprintf(ptr,sizeof(string_to_send)-(ptr-string_to_send),"%d ",attempted_port_arr[i]);
                }
        }
	send_ports_back(string_to_send,&server_browser_port_mapper_ip_cache_entry);

}

static void cleanup_and_send_ports_back(int useless){

	free_attempted_ports(0);
	close_con(&con_obj,0,1);
	exit(useless);

}
static void sigint_handler(int useless){


	innited=useless*0;

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
				raise(SIGINT);
				cleanup_and_send_ports_back(SIGINT);
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
				raise(SIGINT);
				cleanup_and_send_ports_back(SIGINT);
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
	raise(SIGINT);
	cleanup_and_send_ports_back(SIGINT);


}


void init_browser(char* hostname, char* req,uint16_t port){
	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	logging=cfg_server_browser_logging;
	logstream=stdout;

        if(init_addr(&hb_server_addr,hostname,port)){

	    perror("Não conseguimos inicializar address do peer em server_browser!!!\n");
	    raise(SIGINT);
	    cleanup_and_send_ports_back(SIGINT);
	}
	init_con(&con_obj,con_obj.sockfd_tcp,CLIENT_C,our_addr.sin_port,&server_browser_port_mapper_ip_cache_entry);
	int result_con=0;
        uint16_t port_for_us=0;
	while(num_attempted_ports<DEF_DATASIZE){
	        con_obj.sockfd_tcp=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(con_obj.sockfd_tcp<0){
	                perror("Socket nao criada no hb thread!!!\n");
			raise(SIGINT);
	        	cleanup_and_send_ports_back(SIGINT);
	        }

		set_sock_reuseaddr(&con_obj.sockfd_tcp,1);
		ask_for_port(&port_for_us,&server_browser_port_mapper_ip_cache_entry);
		if(!port_for_us||init_addr(&our_addr,server_browser_ip_cache_entry.hostname,port_for_us)){

		    perror("Não conseguimos inicializar address em server_browser!!!\n");
		    raise(SIGINT);
		    cleanup_and_send_ports_back(SIGINT);
		}
		attempted_port_arr[num_attempted_ports]=port_for_us;
		num_attempted_ports++;
		if(bind(con_obj.sockfd_tcp,(struct sockaddr *)&our_addr,socklenvar[1])){

		    perror("Não conseguimos dar bind na socket do server browser!!!\n");
		    print_addr_aux("Este é o address:",&our_addr);
		    raise(SIGINT);
		    cleanup_and_send_ports_back(SIGINT);
		}
		else{

		    print_addr_aux("Bind com sucesso!!!:",&our_addr);
		}

		print_addr_aux("Addr atual do server de heartbeat:",&hb_server_addr);
		if(!(result_con=tryConnect(&con_obj.sockfd_tcp,browser_con_times_pair,&hb_server_addr))){
	        	if(logging){

                                fprintf(logstream,"Initiating forceful teardown!\nResult = %d\n",result_con);
                        }
                        forceful_teardown=1;
                        perror("Nao deu para contactar server de heartbeats!!!!\n");
			raise(SIGINT);
			forceful_teardown=(result_con!=0);
	        	cleanup_and_send_ports_back(SIGINT);
		}
                else if(result_con>0){
                        free_attempted_ports(1);
                        break;
                }
		close_con(&con_obj,0,0);
        }

        getsockname(con_obj.sockfd_tcp,(struct sockaddr*)&our_addr,&socklenvar[1]);

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
			raise(SIGINT);
			cleanup_and_send_ports_back(SIGINT);
        }

	snprintf((char*)con_obj.tcp_data,DEF_DATASIZE-1,"%s",string_to_send);
        int result=con_send_tcp(&con_obj,browser_con_times_pair);
        if(result<0){


                perror("Nao deu para contactar server de heartbeats!!!! Nao recebeu pedido de login\n");
		raise(SIGINT);
		cleanup_and_send_ports_back(SIGINT);
        }
	innited=1;
	recv_servers();


}
