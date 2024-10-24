#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/configs.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/streamer_server.h"


static server_stream_t stream_struct={
					0,
					NULL,
					-1,
					NULL
					};


static void stop_server_stream(server_stream_t* strm){

	if(strm->initted){
        	close_con(strm->con_obj);
        	printf("SAIMOS DA STREAM DO SERVER! Vamos ver errno:%s\n",strerror(errno));
        	close(strm->local_fd);
		strm->initted=0;
	}
	
}

static void sigint_handler(int useless){

	stop_server_stream(&stream_struct +(0*useless));
	exit(useless);
}
static void sigpipe_handler(int useless){

	sigint_handler(useless);
}


static int send_chunk_tcp(server_stream_t* strm,int_pair pair){

	return sendsome(strm->con_obj->sockfd_tcp,(char*)strm->chunk_data_cache,cfg_chunk_size,pair);
        

}
static int send_chunk_udp(server_stream_t* strm,int_pair pair){

	return sendsome_udp(strm->con_obj->sockfd_udp,(char*)strm->chunk_data_cache,cfg_chunk_size,pair,&strm->con_obj->peer_udp_addr);
        

}

static int send_chunk_to_client(server_stream_t* strm,int_pair pair){

        int result=send_chunk_udp(strm, pair);
	int time_to_wait=0;
	char response[cfg_datasize];
	if(result<0){

                return result;
        }
        result=con_read_udp_ack(strm->con_obj,pair);
	if(result<0){

                return result;
        }
	sscanf((char*)strm->con_obj->ack_udp_data,"%s %d",response,&time_to_wait);

	if(time_to_wait >0){

		con_read_udp_ack(strm->con_obj,server_drop_chunks_times_pair);
		result= !(strs_are_strictly_equal((char*)strm->con_obj->ack_udp_data,CON_STRING));
		if(!result){
			printf("Má resposta de cliente!!!! Fechando a conexão!!!\n");
		}
	}
        return result;
}

static void server_stream(server_stream_t* strm){

        while((read(strm->local_fd,strm->chunk_data_cache,cfg_chunk_size)>0)&&(send_chunk_to_client(strm,server_data_times_pair)>0)){}


}

static int init_server_stream(server_stream_t* strm,int fd,con_t* con_obj,unsigned char* stream_buff){
	
	signal(SIGINT, sigint_handler);
	signal(SIGPIPE, sigpipe_handler);
	strm->initted=1;
        strm->con_obj=con_obj;
        strm->local_fd=fd;
	strm->chunk_data_cache=stream_buff;
	memset(strm->chunk_data_cache,0,cfg_chunk_size);
        server_stream(strm);
	raise(SIGINT);
	return 0;
}

void close_stream(void){

	raise(SIGINT);
}

void begin_stream(con_t*con_obj,int fd, unsigned char* stream_buff){

	init_server_stream(&stream_struct,fd,con_obj, stream_buff);

}



