#include "../../Includes/preprocessor.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/streamer_server.h"



static pthread_cond_t running_cond=PTHREAD_COND_INITIALIZER;

static pthread_mutex_t running_mtx=PTHREAD_MUTEX_INITIALIZER,
                 variable_acess_mtx=PTHREAD_MUTEX_INITIALIZER;

static pthread_t tid_stream,
		tid_ack;
atomic_int initted=0;
static struct sigaction sa;


static server_stream_t stream_struct={
					0,
					NULL,
					-1,
					-1,
					NULL,
					NULL,
					0
					};


static void stop_server_stream(server_stream_t* strm){


	if(acess_var_mtx(&variable_acess_mtx,&stream_struct.con_obj->is_on,0,V_LOOK)){
		send_port_back(htons(stream_struct.con_obj->tcp_data_local_port),&server_port_mapper_ip_cache_entry);
		send_ports_back(stream_struct.con_obj);
		close_con(stream_struct.con_obj);
	}
	close(strm->local_fd);
	close(strm->local_fd_boundary);
	pthread_cond_signal(&running_cond);
}

static void cleanup(int useless){
	initted=0*useless;
}
static int send_meta_tcp(server_stream_t* strm,int_pair pair){

	return sendsome(strm->con_obj->sockfd_tcp,(char*)strm->chunk_meta_cache,sizeof(frame_info_t),pair);
        

}
static int send_meta_udp(server_stream_t* strm,int_pair pair){

	return sendsome_udp(strm->con_obj->sockfd_udp,(char*)strm->chunk_meta_cache,sizeof(frame_info_t),pair,&strm->con_obj->peer_udp_addr);
        

}
static int send_chunk_tcp(server_stream_t* strm,int_pair pair){

	return sendsome(strm->con_obj->sockfd_tcp,(char*)strm->chunk_data_cache,strm->chunk_size,pair);
        

}
static int send_chunk_udp(server_stream_t* strm,int_pair pair){

	return sendsome_udp(strm->con_obj->sockfd_udp,(char*)strm->chunk_data_cache,strm->chunk_size,pair,&strm->con_obj->peer_udp_addr);
        

}

static int send_chunk_to_client(void){

	int result=-2;
	if(!is_wav_mode){
	result=send_meta_udp(&stream_struct,server_drop_chunks_times_pair);
	while(initted){
                //result=(server_transmission_protocol<=0)?con_read_tcp(stream_struct.con_obj,server_drop_chunks_ti>
                result=con_read_udp(stream_struct.con_obj,server_drop_chunks_times_pair);
                if(result==-2){
                        printf("Esperando ser respondido na stream do server\n");
			continue;
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

			return result;
		}
	}
	result=(server_transmission_protocol<=0)?send_chunk_tcp(&stream_struct,server_drop_chunks_times_pair):send_chunk_udp(&stream_struct,server_drop_chunks_times_pair);
	while(initted&&(result!=-1)){
		//result=(server_transmission_protocol<=0)?con_read_tcp(stream_struct.con_obj,server_drop_chunks_times_pair):con_read_udp(stream_struct.con_obj,server_drop_chunks_times_pair);
		result=con_read_udp(stream_struct.con_obj,server_drop_chunks_times_pair);
			if(result==-2){
				continue;
			}
			else{
				break;
			}
	}
	return result;
}

static void* server_stream(void* args){
	if(is_wav_mode){
	        while(initted
			&&
			(read(stream_struct.local_fd,stream_struct.chunk_data_cache,stream_struct.chunk_size)>0)
			&&
			(send_chunk_to_client()>0)){


		}
	}
	else{
		printf("We are NOT in wav mode!!!\n");
		while(initted){
			if(read(stream_struct.local_fd_boundary,stream_struct.chunk_meta_cache,sizeof(frame_info_t))<=0){
				fprintf(stderr,"We could not read a frame info thing!\n");
				break;
			}
			else{
				memset(stream_struct.chunk_data_cache,0,server_chunk_size);
			}
			lseek(stream_struct.local_fd,((frame_info_t*)stream_struct.chunk_meta_cache)->start,SEEK_SET);
			if(read(stream_struct.local_fd,stream_struct.chunk_data_cache,((frame_info_t*)stream_struct.chunk_meta_cache)->size)<=0){
				fprintf(stderr,"We could not read a frame using frame info thing!\n");
				break;
			}
			if(send_chunk_to_client()<=0){
				fprintf(stderr,"send a frame obtained using a frame_info_thing!\n");
				break;

			}
		}
	}
	raise(SIGINT);
	stop_server_stream(&stream_struct);
	return args;

}

static void* ack_exchange_thread(void* args){

	int result=0;
	while(initted){
        result=con_read_udp_ack(stream_struct.con_obj,server_data_times_pair);
	if(result<=0){

                if(result==-2){
			stream_struct.curr_timeout++;
			printf("Timeout na stream do server!!!!  timeout %lu de %lu\n",stream_struct.curr_timeout,server_ack_timeout_lim);
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
			printf("Timeout na stream do server!!!!  timeout %lu de %lu\n",stream_struct.curr_timeout,server_ack_timeout_lim);
			if(stream_struct.curr_timeout==server_ack_timeout_lim){
				break;
			}
		}
		perror("");
        }
	}
	raise(SIGINT);
	stop_server_stream(&stream_struct);
	printf("Saimos do thread de acks!!!\n");
	return args;


}
 
static int init_server_stream(int fd,int fd_boundary,con_t* con_obj,uint64_t chunk_size,unsigned char* stream_buff,unsigned char* meta_buff){
	

        sa.sa_handler = cleanup;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGPIPE, &sa, NULL);

	stream_struct.con_obj=con_obj;
        stream_struct.local_fd=fd;
        stream_struct.local_fd_boundary=fd_boundary;
	stream_struct.chunk_size=chunk_size;
	stream_struct.chunk_data_cache=stream_buff;
	stream_struct.chunk_meta_cache=meta_buff;
	memset(stream_struct.chunk_data_cache,0,stream_struct.chunk_size);
	initted=1;
        pthread_create(&tid_ack,NULL,ack_exchange_thread,NULL);
        pthread_create(&tid_stream,NULL,server_stream,NULL);
	
	pthread_mutex_lock(&running_mtx);
	while(initted){

		pthread_cond_wait(&running_cond,&running_mtx);

	}
	pthread_mutex_unlock(&running_mtx);
	
	pthread_join(tid_stream,NULL);
	printf("Saimos do thread de stream!!!\n");
	pthread_join(tid_ack,NULL);
	printf("Saimos do thread de ack!!!\n");
	printf("SAIMOS DA STREAM DO SERVER!\nTimeouts excedidos? %s\nVamos ver errno:%s\n",(stream_struct.curr_timeout==server_ack_timeout_lim) ? "SIM": "NAO",strerror(errno));
	return 0;
}

void close_stream(void){

	raise(SIGINT);
	stop_server_stream(&stream_struct);
}

void begin_stream(con_t*con_obj,int fd, int fd_boundary,uint64_t chunk_size,unsigned char* stream_buff,unsigned char* meta_buff){

	init_server_stream(fd,fd_boundary,con_obj, chunk_size,stream_buff,meta_buff);



}
