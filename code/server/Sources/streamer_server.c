#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/streamer_server.h"



static pthread_cond_t cond1=PTHREAD_COND_INITIALIZER;

static pthread_mutex_t mtx1=PTHREAD_MUTEX_INITIALIZER,
                 mtx4=PTHREAD_MUTEX_INITIALIZER;

static pthread_t tid_stream,
		tid_ack;


static server_stream_t stream_struct={
					0,
					0,
					NULL,
					-1,
					NULL,
					0
					};


static void stop_server_stream(server_stream_t* strm){

	if(acess_var_mtx(&mtx4,&strm->initted,0,V_LOOK)){
        	acess_var_mtx(&mtx4,&strm->initted,0,V_SET);
		close(strm->local_fd);
		close_con(strm->con_obj);
		pthread_cond_signal(&cond1);
	}
	
}

static void cleanup(int useless){
	
	stop_server_stream(&stream_struct + (0*useless));


}
static int send_chunk_tcp(server_stream_t* strm,int_pair pair){

	return sendsome(strm->con_obj->sockfd_tcp,(char*)strm->chunk_data_cache,strm->chunk_size,pair);
        

}
static int send_chunk_udp(server_stream_t* strm,int_pair pair){

	return sendsome_udp(strm->con_obj->sockfd_udp,(char*)strm->chunk_data_cache,strm->chunk_size,pair,&strm->con_obj->peer_udp_addr);
        

}

static int send_chunk_to_client(void){

	int result=-2;
	result=send_chunk_udp(&stream_struct,server_drop_chunks_times_pair);
	while((acess_var_mtx(&mtx4,&stream_struct.initted,0,V_LOOK))&&(result!=-1)){
		result=con_read_udp(stream_struct.con_obj,server_drop_chunks_times_pair);
			if(result==-2){
				printf("Esperando ser respondido na stream do server\n");

			}
			else{
				if(result<0){
					perror("Erro em read na stream do server!!!\n");
				}
				break;
			}
	}
	if(result==-1){
		perror("Erro em send na stream do server!!!\n");

	}
	return result;
}

static void* server_stream(void* args){

        while(acess_var_mtx(&mtx4,&stream_struct.initted,0,V_LOOK)
		&&
		(read(stream_struct.local_fd,stream_struct.chunk_data_cache,stream_struct.chunk_size)>0)
		&&
		(send_chunk_to_client()>0)){


	}
	return args;

}

static void* ack_exchange_thread(void* args){

	int result=0;
	while(acess_var_mtx(&mtx4,&stream_struct.initted,0,V_LOOK)){
        result=con_read_udp_ack(stream_struct.con_obj,server_data_times_pair);
	if(result<=0){

                if(result==-2){
			stream_struct.curr_timeout++;
			printf("Timeout na stream do server!!!!  timeout %hu de %hu\n",stream_struct.curr_timeout,server_ack_timeout_lim);
			if(stream_struct.curr_timeout==server_ack_timeout_lim){
				break;
			}
			continue;
		}
		perror("");
        }
	result= con_send_udp_ack(stream_struct.con_obj,server_data_times_pair);
	if(result<=0){

                if(result==-2){
			stream_struct.curr_timeout++;
			printf("Timeout na stream do server!!!!  timeout %hu de %hu\n",stream_struct.curr_timeout,server_ack_timeout_lim);
			if(stream_struct.curr_timeout==server_ack_timeout_lim){
				break;
			}
		}
		perror("");
        }
	}
	stop_server_stream(&stream_struct);
	printf("Saimos do thread de acks!!!\n");
	return args;


}

static int init_server_stream(int fd,con_t* con_obj,uint16_t chunk_size,unsigned char* stream_buff){
	
	signal(SIGINT,cleanup);
	stream_struct.initted=1;
        stream_struct.con_obj=con_obj;
        stream_struct.local_fd=fd;
	stream_struct.chunk_size=chunk_size;
	stream_struct.chunk_data_cache=stream_buff;
	memset(stream_struct.chunk_data_cache,0,stream_struct.chunk_size);
        pthread_create(&tid_ack,NULL,ack_exchange_thread,NULL);
        pthread_create(&tid_stream,NULL,server_stream,NULL);
	
	pthread_mutex_lock(&mtx1);
	while(acess_var_mtx(&mtx4,&stream_struct.initted,0,V_LOOK)){

		pthread_cond_wait(&cond1,&mtx1);

	}
	pthread_mutex_unlock(&mtx1);
	
	pthread_join(tid_stream,NULL);
	printf("Saimos do thread de stream!!!\n");
	pthread_join(tid_ack,NULL);
	printf("Saimos do thread de ack!!!\n");
	printf("SAIMOS DA STREAM DO SERVER!\nTimeouts excedidos? %s\nVamos ver errno:%s\n",(stream_struct.curr_timeout==server_ack_timeout_lim) ? "SIM": "NAO",strerror(errno));
	return 0;
}

void close_stream(void){

	raise(SIGINT);
}

void begin_stream(con_t*con_obj,int fd, uint16_t chunk_size,unsigned char* stream_buff){

	init_server_stream(fd,con_obj, chunk_size,stream_buff);

}



