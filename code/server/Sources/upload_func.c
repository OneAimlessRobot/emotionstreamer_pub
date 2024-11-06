#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../Includes/configs.h"
#include "../Includes/upload_func.h"



int uploadtofd(int sock,int fd,int_pair times){

char buff[DEF_DATASIZE];
memset(buff,0,DEF_DATASIZE);
int numread;
int all_that_was_sent=0;
int sent=0;
while ((numread = read(fd,buff,DEF_DATASIZE)) > 0) {

        errno=0;
        sent = sendsome(sock, buff,  numread,times);
        memset(buff,0,DEF_DATASIZE);
        /*
	int readjunior=readsome(sock,buff,DEF_DATASIZE,times);
        memset(buff,0,DEF_DATASIZE);
	*/
	int readjunior=0;
        if(sent==-2){

                if(logging){
                fprintf(logstream,"Timeout no sending!!!!: %s\nsocket %d\nLido depois de timeout:%d\n",strerror(errno),sock,readjunior);
                }
                continue;
        }
        if(sent<0){

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if(logging){
                fprintf(logstream,"Block no sending!!!!: %s\nsocket %d\n",strerror(errno),sock);
                }
                break;

        }
        else if(errno==EPIPE){

                if(logging){
                fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
                }
                raise(SIGINT);
                return -1;
        }
        else if(errno == ECONNRESET){
                if(logging){
                fprintf(logstream,"Conexão largada!!\nSIGPIPE!!!!!: %s\n",strerror(errno));
                }
                raise(SIGINT);
                return -1;
        }
        else {
                if(logging){
                fprintf(logstream,"Outro erro qualquer!!!!!: %d %s\n",errno,strerror(errno));
                }
                break;
        }
        }
        	if(logging){
        	fprintf(logstream,"send de %d bytes feito!!!!!\n",sent);
	        }
                all_that_was_sent+=sent;
        }

        if(logging){
        	fprintf(logstream,"send de %d bytes feito!!!!!\n",all_that_was_sent);
	}
	return 0;
}

