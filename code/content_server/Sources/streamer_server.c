#include "../../Includes/preprocessor.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/openssl_stuff.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../Includes/streamer_server.h"





atomic_int initted=0;


static server_stream_t stream_struct={
					0,
					NULL,
					-1,
					-1,
					NULL,
					0
					};


static void stop_server_stream(server_stream_t* strm){


	if(stream_struct.con_obj->is_on){
		close_con(stream_struct.con_obj,0,1);
	}
	close(strm->local_fd);
	close(strm->local_fd_boundary);
}

static void cleanup(int useless){
	initted=0*useless;
}
static int send_chunk(server_stream_t* strm,int_pair pair){
	int result=-1;
	if(strm->con_obj->is_ssl){

		result = sendsome_ssl(strm->con_obj->con_ssl,(char*)strm->chunk_data_cache,strm->chunk_size+(is_wav_mode?0:sizeof(frame_info_t)+4),pair);
	}
	else{
		result = sendsome(strm->con_obj->sockfd_tcp,(char*)strm->chunk_data_cache,strm->chunk_size+(is_wav_mode?0:sizeof(frame_info_t)+4),pair);

	}
	return result;
}

static int send_chunk_to_client(void){

	int result=0;
	while(initted){
		result=send_chunk(&stream_struct,server_drop_chunks_times_pair);
			if(result==-2){
				continue;
			}
			else{
				break;
			}
	}
	return result;
}

static void server_stream(void){
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
			memset(stream_struct.chunk_data_cache,0,sizeof(mp3_stream_chunk));
			if(read(stream_struct.local_fd_boundary,stream_struct.chunk_data_cache,sizeof(frame_info_t))<=0){
				fprintf(stderr,"We could not read a frame info thing!\nError: %s\n",strerror(errno));
				break;
			}
			lseek(stream_struct.local_fd,((mp3_stream_chunk*)(stream_struct.chunk_data_cache))->the_frame_info.start,SEEK_SET);
			if(read(stream_struct.local_fd,stream_struct.chunk_data_cache+sizeof(frame_info_t)+4,((mp3_stream_chunk*)(stream_struct.chunk_data_cache))->the_frame_info.size)<=0){
				fprintf(stderr,"We could not read a frame using frame info thing!\nError: %s\n",strerror(errno));
				break;
			}
			if(send_chunk_to_client()<=0){
				fprintf(stderr,"send a frame obtained using a frame_info_thing!\nError: %s\n",strerror(errno));
				break;

			}
		}
	}
	return;

}

static int init_server_stream(int fd,int fd_boundary,con_t* con_obj,uint64_t chunk_size,unsigned char* stream_buff){


        stream_struct.con_obj=con_obj;
        stream_struct.local_fd=fd;
        stream_struct.local_fd_boundary=fd_boundary;
	stream_struct.chunk_size=chunk_size;
	stream_struct.chunk_data_cache=stream_buff;
	memset(stream_struct.chunk_data_cache,0,stream_struct.chunk_size);
	initted=1;
	server_stream();
	cleanup(0);
	stop_server_stream(&stream_struct);
	printf("SAIMOS DA STREAM DO SERVER!\n");
	return 0;
}

void close_stream(void){

	cleanup(0);
	stop_server_stream(&stream_struct);
}

void begin_stream(con_t*con_obj,int fd, int fd_boundary,uint64_t chunk_size,unsigned char* stream_buff){

	exit_func_for_this_module=close_stream;
	init_server_stream(fd,fd_boundary,con_obj, chunk_size,stream_buff);



}
