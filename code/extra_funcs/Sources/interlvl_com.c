#include "../../Includes/preprocessor.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"
#include "../Includes/openssl_stuff.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/sock_ops.h"
#include "../Includes/ip_cache_file.h"
#include "../Includes/connection.h"
#include "../Includes/server_db_driving.h"
#include "../Includes/interlvl_proto.h"
#include "../Includes/interlvl_com.h"
typedef struct sockaddr_in sockaddr_in_struct;

static void do_indexed_overseer_con_op(int i,overseer_args* arg_s,int is_reply,int reply_result[2]){

	if(!is_reply){
	pthread_mutex_lock(arg_s->cons->set_mtx);
        clear_con_data(&arg_s->cons->con_arr[i]);
        reply_result[0]=con_read(&arg_s->cons->con_arr[i],arg_s->ack_times_pair);
        reply_result[1]=strs_are_strictly_equal((char*)(arg_s->cons->con_arr[i].tcp_data),HB_SEND_STRING);
	pthread_mutex_unlock(arg_s->cons->set_mtx);
	}
	else{
	pthread_mutex_lock(arg_s->cons->set_mtx);
        clear_con_data(&arg_s->cons->con_arr[i]);
        snprintf((char*)(arg_s->cons->con_arr[i].tcp_data),DEF_DATASIZE-1,"%s",HB_REPLY_STRING);
        reply_result[0]=con_send(&arg_s->cons->con_arr[i],arg_s->ack_times_pair);
        pthread_mutex_unlock(arg_s->cons->set_mtx);
        }
}

static void do_indexed_slave_con_op(slave_args* arg_s,int is_reply,int reply_result[2]){
	if(!is_reply){
	pthread_mutex_lock(arg_s->con_mtx);
        clear_con_data(arg_s->con_obj);
        reply_result[0]=con_read(arg_s->con_obj,arg_s->ack_times_pair);
        reply_result[1]=strs_are_strictly_equal((char*)(arg_s->con_obj->tcp_data),HB_REPLY_STRING);
        pthread_mutex_unlock(arg_s->con_mtx);
	}
	else{
	pthread_mutex_lock(arg_s->con_mtx);
        clear_con_data(arg_s->con_obj);
        snprintf((char*)(arg_s->con_obj->tcp_data),DEF_DATASIZE-1,"%s",HB_SEND_STRING);
        reply_result[0]=con_send(arg_s->con_obj,arg_s->ack_times_pair);
        pthread_mutex_unlock(arg_s->con_mtx);
        }
}
void slave_thread_exit_func(int useless,void* ptr){

	if(ptr){
		printf("Slave quit function called!!!\n");
		end_openssl_libs_client_side();
		slave_args* arg_struct=(slave_args*)ptr;
		pthread_mutex_lock(arg_struct->con_mtx);
		send_port_back(htons(arg_struct->this_con_addr.sin_port),&arg_struct->slave_port_mapper_ip_cache_entry);
		if((*arg_struct->loop_var)){
			free_attempted_ports(0,&arg_struct->slave_port_mapper_ip_cache_entry);
		}
		close_con(arg_struct->con_obj,0,1);
		pthread_mutex_unlock(arg_struct->con_mtx);
        	(*arg_struct->start_trigger)=1;
        	pthread_cond_signal(arg_struct->trg_cond);
		arg_struct->sig_func(useless);
		arg_struct->clean_func();
	}


}
void* slave_thread(void* args){
	slave_args* arg_struct= (slave_args*)args;
        print_addr_aux("Addr atual do server de heartbeat:",&arg_struct->master_addr);
	init_openssl_libs_client_side();
	init_con(arg_struct->con_obj,arg_struct->con_obj->sockfd_tcp,CLIENT_C,&arg_struct->slave_port_mapper_ip_cache_entry,arg_struct->is_tls);
	connection_attempt_circuit(&arg_struct->con_obj->sockfd_tcp,slave_thread_exit_func,&arg_struct->this_con_addr,
                                &arg_struct->master_addr,
                                        &arg_struct->slave_ip_cache_entry,
					&arg_struct->slave_port_mapper_ip_cache_entry,
					arg_struct->con_times_pair,
					(void*)(arg_struct));


        setNonBlocking(&(arg_struct->con_obj->sockfd_tcp));
	uint16_t the_port_to_give=htons(arg_struct->this_addr.sin_port);
        clear_con_data(arg_struct->con_obj);
	if(greet(arg_struct->con_obj,arg_struct->con_times_pair)){

		perror("Nao deu para contactar server acima!!!!\nHandshake failed\n");
                slave_thread_exit_func(SIGINT,(void*)(arg_struct));
		return args;

	}
	interlvl_cmd cmd=LOG;
	proto_arr proto_array={0};
	proto_array[0]=(uint16_t)htons(cmd);
	proto_array[1]=(uint16_t)htons(arg_struct->type);
	proto_array[2]=the_port_to_give;
	proto_array[3]=(uint16_t)htons(arg_struct->is_tls);
	memcpy(arg_struct->con_obj->tcp_data,proto_array,PROTO_ARR_SIZE);
	snprintf((char*)&arg_struct->con_obj->tcp_data[PROTO_ARR_SIZE],DEF_DATASIZE-2,"%s %s",arg_struct->extension_buff,arg_struct->lower_name);
        int result=con_send(arg_struct->con_obj,arg_struct->ack_times_pair);
        if(result<0){


                perror("Nao deu para contactar server acima!!!!\nNao recebeu o que mandamos!!!\nNao recebeu pedido de login\n");
                slave_thread_exit_func(SIGINT,(void*)(arg_struct));
		return args;
        }
        result=con_read(arg_struct->con_obj,arg_struct->ack_times_pair);
        if(result<0){


                perror("Nao deu para contactar server acima!!!!\nNao recebemos deles!!!\nNao recebeu pedido de login\n");
                slave_thread_exit_func(SIGINT,(void*)(arg_struct));
		return args;

        }
	(*arg_struct->start_trigger)=1;
        pthread_cond_signal(arg_struct->trg_cond);


        printf("hb_thread do streamer server: online\n");
        while(*arg_struct->loop_var){
	int result[2]={0};
	do_indexed_slave_con_op(arg_struct,1,result);
	if((result[0]<=0)){

                perror("");
		break;
        }
	else if(logging){
		fprintf(logstream,"Ack enviado em slave thread!\n");
        }
	usleep(arg_struct->ack_period_us);
	}
	(*arg_struct->start_trigger)=2;
        slave_thread_exit_func(SIGINT,(void*)(arg_struct));
	printf("Saimos do lower thread\n");
        return args;


}
void init_module_tcp_stuff(int* sockptr,char* addr,uint16_t tcp_s_port,struct sockaddr_in * sockaddr_buff,int exit_signal,int max_connected,int is_port_mapper,ip_cache_entry* port_mapper_cache_entry){


        (*sockptr)= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if((*sockptr)==-1){
		perror("Erro a criar socket em socket listening!!!\n");
                raise(exit_signal);
		exit(-1);

        }
        int ptr=1;
	setsockopt((*sockptr),SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
	uint16_t port=tcp_s_port;
	struct sockaddr_in sockaddr_buff_local={0};
        if(!is_port_mapper){
 		ask_for_port(&port,port_mapper_cache_entry);
	}
	if(!port||init_addr(&sockaddr_buff_local,addr,port)){
		perror("Erro a inicalizar address bind em bootstrapper de listening!!!\n");
		close(*sockptr);
		raise(exit_signal);
		exit(-1);
	}
	if(bind(*sockptr,(struct sockaddr*)(&sockaddr_buff_local),socklenvar[1])){
                if(logging){
			perror("Erro a dar bind em socket de listening!!!\n");
			print_addr_aux("Address em questão:",&sockaddr_buff_local);
		}
		send_port_back(port,port_mapper_cache_entry);
		close(*sockptr);
		raise(exit_signal);
		exit(-1);
        }
	else if(logging){
		print_addr_aux("Sucesso a dar bind!\nAddress em questão:",&sockaddr_buff_local);
	}
        listen(*sockptr,max_connected);
	memcpy(sockaddr_buff,&sockaddr_buff_local,sizeof(struct sockaddr_in));
        if(logging){
		fprintf(logstream,"Listening e bindado!!!\n");
        }
	print_sock_addr(*sockptr);

}
void close_all_fds(con_set* set){

        pthread_mutex_lock(set->set_mtx);
        for(int i=0;i<set->max_size;i++){

                if(set->fd_arr[i]){

                        FD_CLR(set->fd_arr[i],&set->rdfds);
                        FD_ZERO(&set->rdfds);
                        close_con(&set->con_arr[i],0,1);
                        close(set->fd_arr[i]);
                        set->curr_size--;
                        set->fd_arr[i]=0;

                }


        }
        pthread_mutex_unlock(set->set_mtx);

}
static void kill_con(con_set* set,int index){

        pthread_mutex_lock(set->set_mtx);
        close_con(&set->con_arr[index],0,1);
        delete_server(set->fd_arr[index]);
	memset(&set->con_arr[index],0,sizeof(con_t));
        FD_CLR(set->fd_arr[index],&set->rdfds);
        set->fd_arr[index]=0;
        set->curr_size--;
        pthread_mutex_unlock(set->set_mtx);

}



void add_con(con_set* set,con_t*con,char* type_buff,int id,char* name_buff,char* ip_buff,uint16_t stored_port,char* extension_buff,uint16_t using_tls){

        pthread_mutex_lock(set->set_mtx);
	int i=1;
        char big_buff[PATHSIZE*6]={0};
	snprintf(big_buff,sizeof(big_buff)-1,"'%s', %d, %s, '%s:%hu', '%s', %hu",type_buff,id,name_buff,ip_buff,htons(stored_port),extension_buff,using_tls);
        FD_SET(con->sockfd_tcp,&set->rdfds);
        for(;set->fd_arr[i];i++);
        set->fd_arr[i]=con->sockfd_tcp;
        memcpy(&set->con_arr[i],con,sizeof(con_t));
        set->curr_size++;
	insert_server(big_buff);
        pthread_mutex_unlock(set->set_mtx);
	pthread_cond_signal(set->start_cond);
}

void init_con_set(con_set* set,con_t* con_buff,int* fd_buff,int max_size,pthread_mutex_t* mtx,pthread_cond_t* cond){

	set->curr_size=0;
	set->max_size=max_size;
	set->con_arr=con_buff;
	set->fd_arr=fd_buff;
	set->set_mtx=mtx;
	set->start_cond=cond;
}

void* watch_dog_func(void* args){


	overseer_args* arg_s=(overseer_args*)args;

	FD_ZERO(&arg_s->cons->rdfds);

        while(*arg_s->is_on){

        pthread_mutex_lock(arg_s->start_cond_mtx);
        while((*arg_s->is_on)&&!acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->curr_size,0,V_LOOK)){
	       pthread_cond_wait(arg_s->cons->start_cond,arg_s->start_cond_mtx);
        }
        pthread_mutex_unlock(arg_s->start_cond_mtx);

        while((*arg_s->is_on)&&acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->curr_size,0,V_LOOK)){
	usleep(arg_s->ack_period_us);
	for(int i=1;i<arg_s->cons->max_size;i++){
        	int result[2]={0};
        	if(acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->fd_arr[i],0,V_LOOK)){
                	if(FD_ISSET(acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->fd_arr[i],0,V_LOOK),&arg_s->cons->rdfds)){
        			do_indexed_overseer_con_op(i,arg_s,0,result);
				if(result[0]<=0){
			               	perror("");
					kill_con(arg_s->cons,i);
					continue;
				}
				else{
					if(logging){
						fprintf(logstream,"Ack recebido em watchdog thread!\n");
			                }
				}
			}
		}
	}


        }

        }
        arg_s->sig_func(SIGINT);
	return args;


}

module_type string_to_module_type(char*str){
	module_type result=TYPE_NA;
	if(!strs_are_strictly_equal(str,"server_mod")){

		result=SERVER;

	}
	else if(!strs_are_strictly_equal(str,"heartbeat_mod")){

		result=HB_SERVER;

	}
	else if(!strs_are_strictly_equal(str,"master_mod")){

		result=M_SERVER;

	}

	return result;

}
void module_type_to_string(module_type type,char* buff){
	switch(type){
		case SERVER:
			strncpy(buff,"server_mod",strlen("server_mod")+1);
			break;
		case HB_SERVER:
			strncpy(buff,"heartbeat_mod",strlen("heartbeat_mod")+1);
			break;
		case M_SERVER:
			strncpy(buff,"master_mod",strlen("master_mod")+1);
			break;
		default:
			strncpy(buff,"NA_mod",strlen("NA_mod")+1);
	}


}

void* acceptor_func(void* args){
	acceptor_args* arg_a = (acceptor_args*)args;
        char extension_buff[EXTENSION_SIZE+1]={0};
	proto_arr proto_array={0};
	uint16_t master_stored_port=0;
	char req_buff[DEF_DATASIZE+1]={0};
        char ip_buff[PATHSIZE/4]={0};
        char name_buff[PATHSIZE/4]={0};
        char type_buff[PATHSIZE/4]={0};
	int result=0;
	uint16_t useless_arg=0;
        int iResult,
               sock=-1;
	int is_master=((arg_a->arg_s)==NULL);
        printf("Chegamos ao loop de heart beat!\n");
	if(!is_master){
        pthread_mutex_lock(arg_a->master_mtx);
        while(!(*arg_a->started)){
                 printf("Esperando sinal do master server thread!!!\n");
                 pthread_cond_wait(arg_a->arg_s->trg_cond,arg_a->master_mtx);
        }
        pthread_mutex_unlock(arg_a->master_mtx);
        }
	init_openssl_libs_server_side();
        while((*arg_a->is_on)){

                struct timeval tv;
                tv.tv_sec=arg_a->con_times_pair[0];
                tv.tv_usec=arg_a->con_times_pair[1];
                FD_ZERO(&arg_a->mainfds);
                FD_SET(arg_a->accept_sockfd,&arg_a->mainfds);
                iResult=select(arg_a->accept_sockfd+1,&arg_a->mainfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
				con_t con={0};
				sockaddr_in_struct their_addr={0};
        			sock= accept(arg_a->accept_sockfd,NULL,NULL);
				if(sock>=0){

				setNonBlocking(&sock);
				init_con(&con,sock,SERVER_C,&arg_a->acceptor_port_mapper_ip_cache_entry,arg_a->is_tls);
				if(greet(&con,arg_a->con_times_pair)){
					perror("Handshake failed\n");
				        close_con(&con,0,1);
				        continue;

				}
				result=con_read(&con,arg_a->con_times_pair);
				if(result<=0){
				        perror("Nao sabemos o que querem....\n");
				        close_con(&con,0,1);
				        continue;
				}
				interlvl_cmd cmd=(interlvl_cmd)0;

				memcpy(req_buff,con.tcp_data,DEF_DATASIZE);
				memcpy(proto_array,req_buff,PROTO_ARR_SIZE);
				cmd=ntohs(proto_array[0]);
				switch(cmd){

				case MASTER_SHOW:
					if(logging){
						fprintf(logstream,"Anyways....\n....\n....\nShow master requested!!!!\n");
				        }
					if(!is_master){
					snprint_addr_aux(ip_buff,&master_stored_port,PATHSIZE/4,&arg_a->arg_s->master_addr);
					snprintf((char*)con.tcp_data,DEF_DATASIZE-1,"Nao sou um master."
				                                                     "Mas, se quiseres, Está aqui o meu master."
				                                                     "Tenta falar com ele: %s:%hu\n",
				                                                                ip_buff,ntohs(master_stored_port));
					}
					else{
					snprintf((char*)con.tcp_data,DEF_DATASIZE-1,"Sup. Im master. Waddyawant?\n");

					}
					con_send(&con,arg_a->data_times_pair);
				        clear_con_data(&con);
				        snprintf((char*)con.tcp_data,DEF_DATASIZE-1,"done");
				        con_send(&con,arg_a->data_times_pair);
				        close_con(&con,0,1);
				        break;

				case SHOW:
					if(logging){
						fprintf(logstream,"Anyways....\n....\n....\nShow servers requested!!!!\n");
				        }
					show_servers(&con,arg_a->data_times_pair);
				        close_con(&con,0,1);
				        break;
				case LOG:
					getpeername(con.sockfd_tcp, (struct sockaddr*)&their_addr, &socklenvar[1]);
					module_type_to_string((module_type)ntohs(proto_array[1]),type_buff);
					sscanf((char*)&req_buff[PROTO_ARR_SIZE],"%s %s",extension_buff,name_buff);
					result=con_send(&con,arg_a->con_times_pair);
					clear_con_data(&con);
					if(logging){
						fprintf(logstream,"Log server requested!!!!\n");
				        }
					snprint_addr_aux(ip_buff,&useless_arg,PATHSIZE/4,&their_addr);
					add_con(arg_a->arg_o->cons,&con,type_buff,sock,name_buff,ip_buff,ntohs(proto_array[2]),extension_buff,ntohs(proto_array[3]));
				        break;
				default:
					if(logging){
						fprintf(logstream,"Request desconhecido %s!!!!\n",req_buff);
				        }
					close_con(&con,0,1);
				        break;


				}

                        }
                        else{
                             perror("Rejected connection!");
			     perror("Erro no accept no thread de heartbeats!!!!\n");
		             if(errno==EINVAL){

				        arg_a->sig_func(SIGINT);
					arg_a->clean_func();
					break;
				}
			}

                }
                else if(iResult<0){
                        perror("Erro no select no thread de heartbeats!!!!\n");
                        arg_a->sig_func(SIGINT);
			arg_a->clean_func();
			break;
                }
                print_addr_aux("Nada....\n",&arg_a->accept_addr);


        }
        printf("Saimos do thread de heart_beat_master!!!!\n");

	arg_a->sig_func(SIGINT);
        arg_a->clean_func();
	return args;



}
