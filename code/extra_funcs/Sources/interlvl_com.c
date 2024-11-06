#include "../../Includes/preprocessor.h"
#include "../Includes/fileshit.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sock_ops.h"
#include "../Includes/ip_cache_file.h"
#include "../Includes/connection.h"
#include "../Includes/server_db_driving.h"
#include "../Includes/interlvl_proto.h"
#include "../Includes/interlvl_com.h"





void* slave_thread(void* args){
	
	slave_args* arg_struct= (slave_args*)args;

	con_t con_obj= {0};

        con_obj.sockfd_tcp= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);


        if(con_obj.sockfd_tcp<0){
                perror("Socket nao criada no hb thread!!!\n");
                raise(arg_struct->exit_signal);
        }


        print_addr_aux("Addr atual do server de heartbeat:",&arg_struct->master_addr);

        if(!tryConnect(&con_obj.sockfd_tcp,arg_struct->con_times_pair,&arg_struct->master_addr)){

                perror("Nao deu para contactar server de heartbeats!!!!\n");
                raise(arg_struct->exit_signal);
        }
        print_addr_aux("Addr atual do server:",&arg_struct->this_addr);

        init_con(&con_obj,con_obj.sockfd_tcp,CLIENT_C);

	char ent_addr[PATHSIZE/8]={0};

	char mod_type[PATHSIZE/8]={0};

        getsockname(con_obj.sockfd_tcp,(struct sockaddr*)&con_obj.this_tcp_addr,socklenvar);

        greet(&con_obj,arg_struct->con_times_pair,con_obj.this_tcp_addr.sin_port);

	snprint_addr_aux(ent_addr,PATHSIZE/8,&arg_struct->this_addr);

        clear_con_data(&con_obj);
	module_type_to_string(arg_struct->type,mod_type);

        snprintf((char*)con_obj.ack_udp_data,DEF_DATASIZE-1,"%s %s '%s'  %s %hu ",LOG_STRING,mod_type,arg_struct->lower_name,ent_addr,arg_struct->this_addr.sin_port);


        int result=con_send_udp_ack(&con_obj,arg_struct->con_times_pair);
        if(result<0){


                perror("Nao deu para contactar server acima!!!! Nao recebeu pedido de login\n");
                raise(arg_struct->exit_signal);

        }
        result=con_read_udp_ack(&con_obj,arg_struct->con_times_pair);
        if(result<0){


                perror("Nao deu para contactar server acima!!!! Nao recebeu pedido de login\n");
                raise(arg_struct->exit_signal);

        }
        acess_var_mtx(arg_struct->var_mtx,arg_struct->start_trigger,1,V_SET);

        pthread_cond_signal(arg_struct->trg_cond);


        printf("hb_thread do streamer server: online\n");
        uint16_t curr_timeout=0;
        result=0;
        while(acess_var_mtx(arg_struct->var_mtx,arg_struct->loop_var,0,V_LOOK)){

        clear_con_data(&con_obj);
        snprintf((char*)con_obj.ack_udp_data,DEF_DATASIZE-1,"%s",HB_SEND_STRING);
        result=con_send_udp_ack(&con_obj,arg_struct->data_times_pair);
        if(result<=0){

                if(result==-2){
                        curr_timeout++;
                        printf("Timeout no nivel de baixo!!!!  timeout %hu de %hu\n",curr_timeout,arg_struct->ack_timeout_lim);
                        if(curr_timeout==arg_struct->ack_timeout_lim){
                                break;
                        }
                        continue;
                }
                perror("");
        }
        clear_con_data(&con_obj);
        result= con_read_udp_ack(&con_obj,arg_struct->data_times_pair);
        int cmp=strs_are_strictly_equal((char*)con_obj.ack_udp_data,HB_REPLY_STRING);
        if(cmp||(result<=0)){

                if(result==-2){
                        curr_timeout++;
                        printf("Timeout no nivel de baixo!!!!  timeout %hu de %hu\n",curr_timeout,arg_struct->ack_timeout_lim);
                        if(curr_timeout==arg_struct->ack_timeout_lim){
                                break;
                        }
                }
                perror("");
        }
                usleep(arg_struct->sleep_us);

        }
	close_con(&con_obj);
        raise(arg_struct->exit_signal);
        printf("Saimos do lower thread\n");
        return args;


}
void init_module_tcp_stuff(int* sockptr,char* addr,uint16_t tcp_s_port,struct sockaddr_in * sockaddr_buff,int exit_signal,int max_connected){


        (*sockptr)= socket(AF_INET,SOCK_STREAM,0);
        if((*sockptr)==-1){
                raise(exit_signal);

        }
        int ptr=1;
        setsockopt((*sockptr),SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
        init_addr(sockaddr_buff,addr,tcp_s_port);
        if(bind(*sockptr,(struct sockaddr*)(sockaddr_buff),*socklenvar)){
                raise(exit_signal);
                exit(-1);
        }

        listen(*sockptr,max_connected);
        printf("Listening e bindado!!!\n");
        print_sock_addr(*sockptr);

}
void close_all_fds(con_set* set){

        pthread_mutex_unlock(set->set_mtx);
        for(int i=0;i<set->max_size;i++){

                if(set->fd_arr[i]){

                        FD_CLR(set->fd_arr[i],&set->rdfds);
                        FD_ZERO(&set->rdfds);
                        close_con(&set->con_arr[i]);
                        close(set->fd_arr[i]);
                        set->curr_size--;
                        set->fd_arr[i]=0;

                }


        }
        pthread_mutex_unlock(set->set_mtx);

}
static void do_indexed_con_op(int i,overseer_args* arg_s,int is_reply,int reply_result[2]){
	if(!is_reply){
	pthread_mutex_lock(arg_s->cons->set_mtx);
        clear_con_data(&arg_s->cons->con_arr[i]);
        reply_result[0]=con_read_udp_ack(&arg_s->cons->con_arr[i],arg_s->data_times_pair);
        reply_result[1]=strs_are_strictly_equal((char*)arg_s->cons->con_arr[i].ack_udp_data,HB_SEND_STRING);
        pthread_mutex_unlock(arg_s->cons->set_mtx);
	}
	else{
	pthread_mutex_lock(arg_s->cons->set_mtx);
        clear_con_data(&arg_s->cons->con_arr[i]);
        snprintf((char*)arg_s->cons->con_arr[i].ack_udp_data,DEF_DATASIZE-1,"%s",HB_REPLY_STRING);
        reply_result[0]=con_send_udp_ack(&arg_s->cons->con_arr[i],arg_s->data_times_pair);
        pthread_mutex_unlock(arg_s->cons->set_mtx);
        }
}

static void kill_con(con_set* set,int index){

        pthread_mutex_lock(set->set_mtx);
        close_con(&set->con_arr[index]);
        delete_server(set->fd_arr[index]);
	memset(&set->con_arr[index],0,sizeof(con_t));
        FD_CLR(set->fd_arr[index],&set->rdfds);
        set->fd_arr[index]=0;
        set->timeout_arr[index]=0;
        set->curr_size--;
        pthread_mutex_unlock(set->set_mtx);
	
}



void add_con(con_set* set,con_t*con,char* buff){

        int i=1;
        pthread_mutex_lock(set->set_mtx);
        FD_SET(con->sockfd_tcp,&set->rdfds);
        for(;set->fd_arr[i];i++);
        set->fd_arr[i]=con->sockfd_tcp;
        memcpy(&set->con_arr[i],con,sizeof(con_t));
        set->curr_size++;
	insert_server(buff);
        pthread_mutex_unlock(set->set_mtx);
	pthread_cond_signal(set->start_cond);
}

void init_con_set(con_set* set,con_t* con_buff,int* timeout_buff,int* fd_buff,int max_size,pthread_mutex_t* mtx,pthread_cond_t* cond){

	set->curr_size=0;
	set->max_size=max_size;
	set->con_arr=con_buff;
	set->timeout_arr=timeout_buff;
	set->fd_arr=fd_buff;
	set->set_mtx=mtx;
	set->start_cond=cond;
}

void* watch_dog_func(void* args){


	overseer_args* arg_s=(overseer_args*)args;

	FD_ZERO(&arg_s->cons->rdfds);

        while(acess_var_mtx(arg_s->var_mtx,arg_s->is_on,0,V_LOOK)){

        pthread_mutex_lock(arg_s->start_cond_mtx);
        while(acess_var_mtx(arg_s->var_mtx,arg_s->is_on,0,V_LOOK)&&!acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->curr_size,0,V_LOOK)){
	       pthread_cond_wait(arg_s->cons->start_cond,arg_s->start_cond_mtx);
        }
        pthread_mutex_unlock(arg_s->start_cond_mtx);

        while(acess_var_mtx(arg_s->var_mtx,arg_s->is_on,0,V_LOOK)&&acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->curr_size,0,V_LOOK)){

        for(int i=1;i<arg_s->cons->max_size;i++){
        int result[2]={0};
                     	if(acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->fd_arr[i],0,V_LOOK)){
                        if(FD_ISSET(acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->fd_arr[i],0,V_LOOK),&arg_s->cons->rdfds)){
        		do_indexed_con_op(i,arg_s,0,result);
			if(result[1]||(result[0]<=0)){
		
                if(result[0]==-2){
                        uint16_t times=acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->timeout_arr[i],0,V_LOOK);
                        times++;
                        printf("Timeout no heartbeat server!!!!  timeout %hu de %hu\n",times,arg_s->ack_timeout_lim);
                        if(times==arg_s->ack_timeout_lim){
                                kill_con(arg_s->cons,i);
                                break;
                        }
                        acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->timeout_arr[i],times,V_SET);
                       continue;
                }
		else{
                	perror("");
		}
                }
		do_indexed_con_op(i,arg_s,1,result);
		if(result[0]<=0){

                if(result[0]==-2){
                        uint16_t times=acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->timeout_arr[i],0,V_LOOK);
                        times++;
                        printf("Timeout no heartbeat server!!!!  timeout %hu de %hu\n",times,arg_s->ack_timeout_lim);
                        if(times==arg_s->ack_timeout_lim){
                                kill_con(arg_s->cons,i);
                                break;
                        }
                        acess_var_mtx(arg_s->cons->set_mtx,&arg_s->cons->timeout_arr[i],times,V_SET);
                }
		else{
                	perror("");
		}
		}

                        }
        }
}



        }

        }
        raise(arg_s->exit_signal);
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