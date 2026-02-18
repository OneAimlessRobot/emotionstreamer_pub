#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sockio_tcp.h"
#include <openssl/ssl.h>
#include "../Includes/fileshit.h"


int sendsome(int sd,char buff[],size_t size,int_pair times){
                if(sd>=0){
			int iResult;
	                struct timeval tv;
	                size_t send_total=0;
			ssize_t s=0;
			while(send_total<size){
				fd_set wfds;
		                FD_ZERO(&wfds);
		                FD_SET(sd,&wfds);
		                tv.tv_sec=times[0];
		                tv.tv_usec=times[1];
		                iResult=select(sd+1,(fd_set*)0,&wfds,(fd_set*)0,&tv);
		                if(iResult>0){

		                send_total+= (s=send(sd,buff+send_total,size-send_total,0));
				if (s < 0) return -1;
	        		if (s == 0) return send_total;
		                }
				else if(!iResult){
		               	return -2;
				}
				else{
				if(logging){

				fprintf(logstream, "SELECT ERROR!!!!! SEND\n%s\n",strerror(errno));
				}
				return -1;
				}
			}
			return send_total;
		}
		return -1;
}

int sendallfd(int sock,int fd,int_pair times){

char buff[DEF_DATASIZE];
int numread;
int sent=0;
while ((numread = read(fd,buff,DEF_DATASIZE)) > 0) {
    
    int totalsent = 0;
    while (totalsent < numread) {
        errno=0;
	sent = sendsome(sock, buff + totalsent,  numread - totalsent,times);
	if(sent==-2){

		if(logging){
		fprintf(logstream,"Timeout no sending!!!!: %s\nsocket %d\n",strerror(errno),sock);
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
	
		raise(SIGINT);
		break;
	}
	}
	totalsent += sent;
	}
}

return 0;
}

int readsome(int sd,char buff[],size_t size,int_pair times){
		if(sd>=0){
			int iResult;
	                struct timeval tv;
			size_t read_total=0;
			ssize_t r=0;
			while(read_total<size){
	                fd_set rfds;
	                FD_ZERO(&rfds);
	                FD_SET(sd,&rfds);
	                tv.tv_sec=times[0];
	                tv.tv_usec=times[1];
	                iResult=select(sd+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
	                if(iResult>0){

	                read_total+= (r=recv(sd,buff+read_total,size-read_total,0));
			if (r < 0) return -1;
	        	if (r == 0) return read_total;
	                }
			else if(!iResult){
	               	return -2;
			}
			else{
			if(logging){

			fprintf(logstream, "SELECT ERROR!!!!! READ\n%s\n",strerror(errno));
			}
			return -1;
			}
			}
			return read_total;
		}
		return -1;
}


int sendall(int sock,char buff[],size_t size,int_pair times){
        int len=0;
	size_t total=0;
	for(len=sendsome(sock,buff+total,size-total,times);(len>0)&&(total!=size);total+=len){
	
			len=sendsome(sock,buff+total,size-total,times);
	}
	
	if(!(total-size)){
		if(logging){
		fprintf(logstream,"sendall bem sucedido!! A socket e %d\n",sock);
		
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
		fprintf(logstream,"sendall saiu com erro!!!!!:\nAvisando server para desconectar!\n%s\n",strerror(errno));
		}
		
		return -2;
	}
	else if(len!=-2){
		if(logging){
		fprintf(logstream,"sendall saiu com erro!!!!!:\n%s\n",strerror(errno));
		}
	}
	
        return total;

}

int readall(int sock,char buff[],size_t size,int_pair times){
        int len=0;
	size_t total=0;
	for(len=readsome(sock,buff+total,size-total,times);(len>0)&&(total!=size);total+=len){
	
			len=readsome(sock,buff+total,size-total,times);
	}
	
	if(!(total-size)){
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
	
        return total;

}

int readalltofd(int sock,int fd,size_t size,int_pair times){
        int32_t len=1;
	int32_t written=1;
	size_t total=0;
	char buff[DEF_DATASIZE];
	memset(buff,0,DEF_DATASIZE);
	for(;(len==-2||len>0)&&(total!=size);){
                len=readsome(sock,buff,DEF_DATASIZE,times);
                written=write(fd,buff,len);
                total+= (written<0)? 0:written;
                memset(buff,0,DEF_DATASIZE);
        }

	if(!(total-size)){
		if(logging){
		fprintf(logstream,"readall bem sucedido!! A socket e %d\n",sock);

		}
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
		fprintf(logstream,"readalltofd bem sucedido. A socket e %d\nLemos %lu de %lu bytes\n",sock,total,size);

	}
	memset(buff,0,DEF_DATASIZE);
	
        return 0;

}

