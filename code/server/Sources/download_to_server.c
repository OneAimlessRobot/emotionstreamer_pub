#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../Includes/download_to_server.h"




int download_to_server_func(int sock,int fd,int64_t size,int_pair times){
        int64_t len=1;
        int64_t total=0;
        char buff[DEF_DATASIZE];
        memset(buff,0,DEF_DATASIZE);
        for(;(len==-2||len>0)&&(total!=size);){
                len=readsome(sock,buff,DEF_DATASIZE,times);
                if(len==-2){
                        printf("Timeout no download!!!\n");
                }
                write(fd,buff,len);
                memset(buff,0,DEF_DATASIZE);
        }
        if(len<0){
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if(logging){
                fprintf(logstream,"readall bem sucedido!! A socket e %d\n",sock);
                }
        }
        else if(errno==EPIPE){

                if(logging){
	                fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
                }
                return -2;
        }
        else if(errno==ENOTCONN){
                if(logging){
                fprintf(logstream,"readall saiu com erro!!!!!:\nAvisando server para desconectar!\n%s\n",strerror(errno));
                }

                return -2;
        }
        else if(len!=-2){
                if(logging){
                fprintf(logstream,"readall saiu com erro!!!!!:\n%s\n",strerror(errno));
                }
        }

        }
        if(logging){
                fprintf(logstream,"readalltofd bem sucedido. A socket e %d\nLemos %ld bytes\n",sock,size);

        }
        memset(buff,0,DEF_DATASIZE);
        sendsome(sock,buff,DEF_DATASIZE,times);

        return 0;

}
