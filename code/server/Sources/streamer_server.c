#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sockio_udp.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/streamer_server.h"


static server_stream_t stream_struct={
					NULL,
					-1,
					{0}
					};


static void stop_server_stream(server_stream_t* strm){

        close_con(strm->con_obj);
        printf("SAIMOS DA STREAM DO SERVER! Vamos ver errno:%s\n",strerror(errno));
        close(strm->local_fd);
}

static int send_chunk_to_client(server_stream_t* strm,int_pair pair){

        int result=sendsome_udp(strm->con_obj->sockfd_udp,(char*)strm->chunk_data_cache,CHUNK_SIZE,pair,&strm->con_obj->peer_udp_addr);
        int time_to_wait=0;
	char response[DEF_DATASIZE];
	if(result<0){

                stop_server_stream(strm);

        }
        result=con_read_tcp(strm->con_obj,pair);
	if(result<0){

                stop_server_stream(strm);

        }
	sscanf((char*)strm->con_obj->data,"%s %d",response,&time_to_wait);
	
	if(time_to_wait >0){

		usleep(time_to_wait*1000);
		//con_read_tcp(strm->con_obj,SERVER_DROP_CHUNK_TIMES_PAIR);
	}
        return result;
}

static void server_stream(server_stream_t* strm){

        while((read(strm->local_fd,strm->chunk_data_cache,CHUNK_SIZE)>0)&&(send_chunk_to_client(strm,SERVER_DATA_TIMES_PAIR)>0)){}

}

static int init_server_stream(server_stream_t* strm,int fd,con_t* con_obj){


        strm->con_obj=con_obj;
        strm->local_fd=fd;
        server_stream(strm);
        stop_server_stream(strm);
        return 0;
}

void close_stream(void){

	stop_server_stream(&stream_struct);
}

void begin_stream(con_t*con_obj,int fd){

	init_server_stream(&stream_struct,fd,con_obj);


}



