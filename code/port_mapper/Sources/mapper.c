#include "../../Includes/preprocessor.h"
#include <ncurses.h>
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/interlvl_com.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include "../Includes/mapper.h"
#include "../Includes/configs.h"

static pthread_mutex_t running_mtx=PTHREAD_MUTEX_INITIALIZER,
			input_mtx=PTHREAD_MUTEX_INITIALIZER,
			con_mtx=PTHREAD_MUTEX_INITIALIZER,
			variable_mtx=PTHREAD_MUTEX_INITIALIZER;


static pthread_cond_t running_cond=PTHREAD_COND_INITIALIZER,
			input_cond=PTHREAD_COND_INITIALIZER;

static pthread_t input_tid=0,
		main_tid=0;

static int input_enabled=0;
atomic_int running=ATOMIC_VAR_INIT(0);
static struct sigaction sa;

static port_mapper mapper={NULL,0,-1,{0},{{0}}};



static void sigterm_handler(int useless){


	running=0*useless;

}
static void sigint_handler(int useless){

	running=0*useless;
}

static void cleanup(void){

	close(mapper.socket);
	pthread_cond_signal(&running_cond);
	pthread_cond_signal(&input_cond);


}
static int port_in_range(int port){

	return (port>=cfg_init_port)&&(port<=(cfg_init_port+cfg_num_ports));


}
static int is_no_more_room(void){

	
	return (acess_var_mtx(&running_mtx,&mapper.curr_num_ports,0,V_LOOK)>=cfg_num_ports);


}
static int is_empty(void){

	
	return !acess_var_mtx(&running_mtx,&mapper.curr_num_ports,0,V_LOOK);


}

static void close_ports(int arr[NUM_PORTS_TO_GIVE+1]){

	if(!is_empty()){
		for(int i=0;i<arr[0];i++){
			if(acess_var_mtx(&variable_mtx,&(mapper.port_arr[arr[i+1]-cfg_init_port]),0,V_LOOK)==PORT_ALLOCATED){
				acess_var_mtx(&variable_mtx,&(mapper.port_arr[arr[i+1]-cfg_init_port]),0,V_SET);
				acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,0,V_LOOK)-1,V_SET);
			}

		}
	}


}
static int close_single_port(int port){

	if(!is_empty()){
		if(acess_var_mtx(&variable_mtx,&(mapper.port_arr[port-cfg_init_port]),0,V_LOOK)==PORT_ALLOCATED){
			acess_var_mtx(&variable_mtx,&(mapper.port_arr[port-cfg_init_port]),0,V_SET);
			acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,0,V_LOOK)-1,V_SET);
			return 1;
		}
	}
	return 0;


}
static int fetch_ports_to_give(int arr[NUM_PORTS_TO_GIVE+1],int actually_change){

	int init=cfg_init_port;
	int curr=cfg_init_port;
	int final=cfg_init_port+cfg_num_ports;
	if(!is_no_more_room()){
		for(curr=init;(curr<final)&&(arr[0]<NUM_PORTS_TO_GIVE);curr++){
			if(acess_var_mtx(&variable_mtx,&(mapper.port_arr[curr-init]),0,V_LOOK)==PORT_FREE){
				arr[arr[0]+1]=(uint16_t)curr;
				acess_var_mtx(&variable_mtx,&(mapper.port_arr[curr-init]),1*actually_change,actually_change?V_SET:V_LOOK);
				acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,0,V_LOOK)+(1*actually_change),actually_change?V_SET:V_LOOK);
				arr[0]++;
			}

		}
	}
	int result=arr[0]!=NUM_PORTS_TO_GIVE;

	if(result){
		close_ports(arr);


	}
	return (arr[0]!=NUM_PORTS_TO_GIVE);


}
static int fetch_single_port_to_give(int* port,int actually_change){

	int init=cfg_init_port;
	int curr=cfg_init_port;
	int final=cfg_init_port+cfg_num_ports;
	if(!is_no_more_room()){
		for(curr=init;(curr<final);curr++){
			if(acess_var_mtx(&variable_mtx,&(mapper.port_arr[curr-init]),0,V_LOOK)==PORT_FREE){
				port[0]=(uint16_t)curr;
				acess_var_mtx(&variable_mtx,&(mapper.port_arr[curr-init]),1*actually_change,actually_change?V_SET:V_LOOK);
				acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,0,V_LOOK)+(1*actually_change),actually_change?V_SET:V_LOOK);
				return 1;
			}

		}
	}
	
	return 0;

}
static int reserve_port(int port){

	if(port_in_range(port)){
		if(!is_no_more_room()){
			if(acess_var_mtx(&variable_mtx,&(mapper.port_arr[port-cfg_init_port]),0,V_LOOK)!=PORT_ALLOCATED){
				acess_var_mtx(&variable_mtx,&(mapper.port_arr[port-cfg_init_port]),PORT_RESERVED,V_SET);
				acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,0,V_LOOK)+1,V_SET);
				return 1;
			}

		}
	}
	return 0;

}
static int unreserve_port(int port){

	if(port_in_range(port)){
		if(!is_empty()){
			if(acess_var_mtx(&variable_mtx,&(mapper.port_arr[port-cfg_init_port]),0,V_LOOK)<0){
				acess_var_mtx(&variable_mtx,&(mapper.port_arr[port-cfg_init_port]),PORT_FREE,V_SET);
				acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,acess_var_mtx(&variable_mtx,&mapper.curr_num_ports,0,V_LOOK)-1,V_SET);
				return 1;
			}

		}
	}
	return 0;


}

static void port_mapper_print(int fd){


	char buff[4096+cfg_num_ports+4096];
	memset(buff,0,4096+cfg_num_ports+4096);
	char* ptr=buff,*prev_ptr;
	prev_ptr=ptr+=snprintf(ptr,sizeof(buff),"Aqui está o estado atual do port mapper!\nPortas a fornecer: de %d a %d\n\n\n",cfg_init_port,cfg_init_port+cfg_num_ports);
	prev_ptr=ptr+=snprintf(ptr,sizeof(buff),"%sReservadas: 'r'\nAllocadas: 't'\nLivres: '-'\n[",prev_ptr);
	for(int i=0;i<cfg_num_ports;i++){
		int port_state=acess_var_mtx(&variable_mtx,&(mapper.port_arr[i]),0,V_LOOK);
		ptr[i]=(port_state?((port_state>0)?'t':'r'):'-');

	}
	prev_ptr=ptr+=cfg_num_ports;
	int arr[NUM_PORTS_TO_GIVE+1]={0};
	fetch_ports_to_give(arr,0);
	prev_ptr=ptr+=snprintf(ptr,sizeof(buff),"%s]\n\nAqui estão as portas que seriam entregadas a seguir:\nSeriam entregues %d portas!\n",prev_ptr,arr[0]);

	for(int i=1;i<=NUM_PORTS_TO_GIVE;i++){
	prev_ptr=ptr+=snprintf(ptr,sizeof(buff),"%s-%d\n",prev_ptr,arr[i]);

	}
	prev_ptr=ptr+=snprintf(ptr,sizeof(buff),"%s\n\n"
						"\nPort mapper está a correr? %s"
						"\nEstamos cheios? %s"
						"\nEstamos vazios? %s"
						"\nNumero atual de portas:%d\n\n",
						prev_ptr,
						running?"Yes!":"No...",
						is_no_more_room()?"Yes!":"No....",
						is_empty()?"Yes!":"No....",
						acess_var_mtx(&running_mtx,&mapper.curr_num_ports,0,V_LOOK));
	
	dprintf(fd,"%s",buff);

}
static void print_help(void){

	printf("Listagem de ajuda:"
				"\n- 'c' <port> -> Verificar estado de uma porta"
					"\n- 'p' -> Imprimir estado do mapper"
					"\n- 's' -> Parar mapper"
					"\n- 'h' -> Imprimir este menu\n");


}
static void send_ports_to_client(int sock,int port_arr[NUM_PORTS_TO_GIVE+1]){

	char buff_with_the_ports[4096]={0};
	char* ptr= buff_with_the_ports,*prev_ptr;
	prev_ptr=ptr;

	for(int i=1;i<NUM_PORTS_TO_GIVE+1;i++){
	

		prev_ptr=ptr+=snprintf(ptr,sizeof(buff_with_the_ports)-1,"%s %d",prev_ptr,port_arr[i]);


	}
	int result=sendsome(sock,buff_with_the_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("Portas não enviadas!!! %s\n",buff_with_the_ports);
		

	}
	else{

		printf("Portas enviadas!!! %s\n",buff_with_the_ports);


	}
	memset(buff_with_the_ports,0,4096);
        result=readsome(sock,buff_with_the_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
                 fprintf(stderr,"Não conseguimos receber portas do port mapper!!!!!!\nString que recebemos: \"%s\"\nError string: %s\n",buff_with_the_ports,strerror(errno));
		 close(sock);
                 raise(SIGTERM);

        }
	else{
	         fprintf(stdout,"Não conseguimos receber portas do port mapper!!!!!!\nString que recebemos: \"%s\"\n",buff_with_the_ports);
        }
	close(sock);


}

static void close_ports_from_client(int sock,char* ports_and_info_buff,int port_arr[NUM_PORTS_TO_GIVE+1]){

	int result=sendsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("timeout no port mapper");
		close(sock);
		return;
	}
	memset(ports_and_info_buff,0,4096);
	result=readsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("timeout no port mapper");
		close(sock);
		return;
	}
	port_arr[0]=2;
	sscanf(ports_and_info_buff,"%d %d",&port_arr[1],&port_arr[2]);
	printf("Recebemos estas portas para fechar!!!\n%d e %d\n",port_arr[1],port_arr[2]);
	close_ports(port_arr);
	close(sock);

}

static void check_port_func(uint16_t port_to_check){


	if((port_to_check<cfg_init_port) || (port_to_check>(cfg_init_port+cfg_num_ports))){

		printf("Port out of range!\nRange is between %d and %d\n",cfg_init_port,cfg_init_port+cfg_num_ports);
	}
	else{
		int port_state=acess_var_mtx(&variable_mtx,&(mapper.port_arr[port_to_check-cfg_init_port]),0,V_LOOK);
        	char str[4096]={0};
		snprintf(str,sizeof(str)-1,"%s",(port_state?((port_state>0)?"Taken":"Reserved"):"Free"));
		printf("%s\n",str);

	}


}
static void reserve_client_port(int sock,char* ports_and_info_buff){
	int result=-1;
	memset(ports_and_info_buff,0,4096);
	result=sendsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("timeout no mapper a avisar ao cliente que estamos prontos para reservar portas!!!\n");
		close(sock);
	}
	memset(ports_and_info_buff,0,4096);
	result=readsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		printf("Timeout no port mapper a reservar portas do cliente?!!!!\n");
		close(sock);
	}
	int reserved_port=0;
	sscanf(ports_and_info_buff,"%d",&reserved_port);
	printf("Eles querem reservar a porta %d\n",reserved_port);
	memset(ports_and_info_buff,0,4096);
	int result_of_reserve=reserve_port(reserved_port);
	snprintf(ports_and_info_buff,DEF_DATASIZE,"%d",result_of_reserve);
	result=sendsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		printf("Cliente não apanhou o nosso aviso!!!\n");
	}
	else if(!result_of_reserve){
		printf("Não foi possivel reservar porta!\n");
	}
	else{
		printf("Reserva feita!\n");
	}
	close(sock);

}
static void send_single_client_port(int sock,int* port){
	
	char buff_with_the_ports[4096]={0};

	snprintf(buff_with_the_ports,sizeof(buff_with_the_ports)-1,"%d",port[0]);

	int result=sendsome(sock,buff_with_the_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("Portas não enviadas!!! %s\n",buff_with_the_ports);
		

	}
	else{

		printf("Porta enviada!!! %s\n",buff_with_the_ports);


	}
	memset(buff_with_the_ports,0,4096);
        result=readsome(sock,buff_with_the_ports,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
                 fprintf(stderr,"Não conseguimos receber porta do port mapper!!!!!!\nString que recebemos: \"%s\"\nError string: %s\n",buff_with_the_ports,strerror(errno));
		 close(sock);
                 raise(SIGTERM);

        }
	else{
		fprintf(stdout,"conseguimos receber porta do port mapper!!!!!!\nString que recebemos: \"%s\"\n",buff_with_the_ports);
                 
	}

}
static void close_single_port_from_client(int sock,char* ports_and_info_buff,int* port){

	int result=sendsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("timeout no port mapper");
		close(sock);
		return;
	}
	memset(ports_and_info_buff,0,4096);
	result=readsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("timeout no port mapper");
		close(sock);
		return;
	}
	sscanf(ports_and_info_buff,"%d",port);
	printf("Recebemos esta porta para fechar!!!\n%d\n",port[0]);
	close_single_port(port[0]);
	close(sock);

}
static void unreserve_client_port(int sock,char* ports_and_info_buff){
	int result=-1;
	memset(ports_and_info_buff,0,4096);
	result=sendsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){

		printf("timeout no mapper a avisar ao cliente que estamos prontos para desreservar portas!!!\n");
		close(sock);
	}
	memset(ports_and_info_buff,0,4096);
	result=readsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		printf("Timeout no port mapper a desreservar portas do cliente?!!!!\n");
		close(sock);
	}
	int reserved_port=0;
	sscanf(ports_and_info_buff,"%d",&reserved_port);
	printf("Eles querem desreservar a porta %d\n",reserved_port);
	memset(ports_and_info_buff,0,4096);
	int result_of_reserve=unreserve_port(reserved_port);
	snprintf(ports_and_info_buff,DEF_DATASIZE,"%d",result_of_reserve);
	result=sendsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	if(result<=0){
		printf("Cliente não apanhou o nosso aviso!!!\n");
	}
	else if(!result_of_reserve){
		printf("Não foi possivel desreservar porta!\n");
	}
	else{
		printf("Desreserva feita!\n");
	}
	close(sock);

}
void* port_mapper_input_loop(void* args){

	
        pthread_mutex_lock(&input_mtx);
        while(running&&!acess_var_mtx(&variable_mtx,&input_enabled,0,V_LOOK)){

                pthread_cond_wait(&input_cond,&input_mtx);
        }
        pthread_mutex_unlock(&input_mtx);
	printf("Thread de input do port mapper acordou!\n");
	
	while(running){
  		
		char string[128]={0};
		uint16_t port_to_check=0;
		scanf("%s",string);
		fflush(stdin);
		command_char c = (command_char) string[0];
		switch(c){
			case CHECK_PORT:
				scanf("%hu",&port_to_check);
				fflush(stdin);
				check_port_func(port_to_check);
				break;
			case PRINT_STUFF:
				port_mapper_print(1);
				break;
			case STOP_MAPPER:
				raise(SIGINT);
				break;
			case PRINT_HELP:
				print_help();
				break;
			default:
				printf("Hotel?.......\nTrivago....\nNão é? Eu... Eu não sei, memo...\n");
				break;
		}

	}


	return args;

}
static void* accepted_connection_thread(void* args){
	

	int sock= ((int*)args)[0];
	pthread_mutex_lock(&con_mtx);
	setNonBlocking(sock);
	char request_buff[4096]={0};
	char ports_and_info_buff[4096]={0};
	int ports_to_work_with[NUM_PORTS_TO_GIVE+1]={0};
	int port_to_work_with[1]={0};
	readsome(sock,ports_and_info_buff,DEF_DATASIZE,port_mapper_times_pair);
	sscanf(ports_and_info_buff,"%s",request_buff);
	fflush(stdout);
	printf("Recebemos esta string de request: '%s'\n",request_buff);
	if(!strs_are_strictly_equal(request_buff,PORT_MAPPER_JOIN_STRING)){

		fetch_ports_to_give(ports_to_work_with,1);
		send_ports_to_client(sock,ports_to_work_with);
		close(sock);
	}
	else if(!strs_are_strictly_equal(request_buff,PORT_MAPPER_AWKWARD_JOIN_STRING)){

		fetch_single_port_to_give(port_to_work_with,1);
		send_single_client_port(sock,port_to_work_with);
		close(sock);
	}
	else if(!strs_are_strictly_equal(request_buff,PORT_MAPPER_RESERVE_STRING)){
		reserve_client_port(sock,ports_and_info_buff);
		close(sock);
	}
	else if(!strs_are_strictly_equal(request_buff,PORT_MAPPER_LEAVE_STRING)){
		close_ports_from_client(sock,ports_and_info_buff,ports_to_work_with);
		close(sock);
	}
	else if(!strs_are_strictly_equal(request_buff,PORT_MAPPER_AWKWARD_LEAVE_STRING)){
		close_single_port_from_client(sock,ports_and_info_buff,port_to_work_with);
		close(sock);
	}
	else if(!strs_are_strictly_equal(request_buff,PORT_MAPPER_UNRESERVE_STRING)){
		unreserve_client_port(sock,ports_and_info_buff);
		close(sock);
	}
	else{
		printf("??????????\n");
		close(sock);
	}
	pthread_mutex_unlock(&con_mtx);
	return args;

}
void* port_mapper_main_loop(void* args){

	
	while(running){
		 struct timeval tv;
		 int sock=-1;
                 tv.tv_sec=port_mapper_times_pair[0];
                 tv.tv_usec=port_mapper_times_pair[1];
                 FD_ZERO(&mapper.con_fds);
                 FD_SET(mapper.socket,&mapper.con_fds);
                 int iResult=select(mapper.socket+1,&mapper.con_fds,(fd_set*)0,(fd_set*)0,&tv);
                 if(iResult>0){
		 	 sock= accept(mapper.socket,NULL,NULL);
                         if(sock>=0){
				printf("Connection accepted!\n");
                        	int arg[1]={0};
				arg[0]=sock;
				/*pthread_t tid_con=-1;
				pthread_create(&tid_con,NULL,accepted_connection_thread,(void*)arg);
				pthread_detach(tid_con);
				*/
				accepted_connection_thread((void*)arg);
				
			}
		}
		else if(!iResult)
		{
			printf("Timeout no port mapper!!!\n");

		}
		else{

		       perror("Select error no port mapper!!!!\n");
		       raise(SIGINT);
		}
		pthread_cond_signal(&input_cond);
	}


	return args;

}




void port_mapper_init(ip_cache_entry* ent){

	signal(SIGINT,sigint_handler);
	sa.sa_handler = sigint_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

	int32_t port_arr[cfg_num_ports];
	memset(port_arr,0,sizeof(int32_t)*cfg_num_ports);
	mapper.port_arr=port_arr;



	init_addr(&mapper.addr_struct, ent->hostname,ent->port);
	init_module_tcp_stuff(&mapper.socket,ent->hostname,ent->port,&mapper.addr_struct,SIGTERM,cfg_num_ports,1,NULL);
	running=1;
	input_enabled=1;
        pthread_create(&input_tid,NULL,port_mapper_input_loop,NULL);

        pthread_create(&main_tid,NULL,port_mapper_main_loop,NULL);

	pthread_mutex_lock(&running_mtx);
        while(running){

                pthread_cond_wait(&running_cond,&running_mtx);
        }
        pthread_mutex_unlock(&running_mtx);
	cleanup();
	pthread_join(input_tid ,NULL);
	printf("Saimos do thread de input!\n");
	pthread_join(main_tid ,NULL);
	printf("Saimos do main thread!\n");
	exit(0);

}
