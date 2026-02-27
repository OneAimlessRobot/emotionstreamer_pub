#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/fileshit.h"


int sendsome_ssl(SSL* ssl, const char* buf, size_t len, int_pair times) {

	int sd= SSL_get_fd(ssl);
	size_t send_total = 0;
	while (send_total < len) {
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set wrfds;
	FD_ZERO(&wrfds);
	FD_SET(sd, &wrfds);
	int iResult=select(sd + 1, (fd_set*)0, &wrfds, (fd_set*)0, &tv);
	if(iResult>0){
		int ret = SSL_write(ssl, buf + send_total, len - send_total);
	        if (ret > 0) {
	            send_total += ret;
	            continue;
	        }
		else if (ret == 0) {
			return send_total;
		}
		int ssl_err = SSL_get_error(ssl, ret);
		if (ssl_err == SSL_ERROR_WANT_READ) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mrfds;
			FD_ZERO(&mrfds);
			FD_SET(sd, &mrfds);
			select(sd + 1, &mrfds, (fd_set*)0, (fd_set*)0, &mtv);
			continue;
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mwfds;
			FD_ZERO(&mwfds);
			FD_SET(sd, &mwfds);
			select(sd + 1, (fd_set*)0, &mwfds, (fd_set*)0, &mtv);
			continue;
		}
		else if (ssl_err == SSL_ERROR_ZERO_RETURN) {
			return send_total;
		}
		else{
		    if(errno==EAGAIN){

			continue;
		    }
		    ERR_print_errors_fp(stderr);
	            if(logging){

			fprintf(logstream, "SSL ERROR AT SSL SEND\n%s\n",strerror(errno));
		    }
		    if(use_exit_func){
			exit_func_for_this_module();
		    }
		    return -1;
		}
	}
	else if(!iResult){
		return -2;
	}
	else{
		if(logging){

	        	fprintf(logstream, "SELECT ERROR!!!!! SSL SEND\n%s\n",strerror(errno));
		}
		if(use_exit_func){
			exit_func_for_this_module();
		}
		return -1;
	}

}
return send_total;


}

int readsome_ssl(SSL* ssl, char* buf, size_t len, int_pair times) {
	int sd= SSL_get_fd(ssl);
	size_t read_total = 0;
	while (read_total < len) {
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sd, &rfds);
	int iResult=select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
	if(iResult>0){
		int ret = SSL_read(ssl, buf + read_total, len - read_total);
		if (ret > 0) {
			read_total += ret;
			continue;
		}
		else if (ret == 0) {
			return read_total;
		}
		int ssl_err = SSL_get_error(ssl, ret);
		if (ssl_err == SSL_ERROR_WANT_READ) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mrfds;
			FD_ZERO(&mrfds);
			FD_SET(sd, &mrfds);
			select(sd + 1, &mrfds, (fd_set*)0, (fd_set*)0, &mtv);
			continue;
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set wfds;
			FD_ZERO(&wfds);
			FD_SET(sd, &wfds);
			select(sd + 1, (fd_set*)0, &wfds, (fd_set*)0, &mtv);
			continue;
		}
		else if (ssl_err == SSL_ERROR_ZERO_RETURN) {
			return read_total;
		}
		else{
		    if(errno==EAGAIN){

			continue;
		    }
		    ERR_print_errors_fp(stderr);
	            if(logging){

			fprintf(logstream, "SSL ERROR AT SSL READ\n%s\n",strerror(errno));
		    }
		    if(use_exit_func){
			exit_func_for_this_module();
		    }
		    return -1;
		}
	}
	else if(!iResult){
		return -2;
	}
	else{
		if(logging){

	        	fprintf(logstream, "SELECT ERROR!!!!! SSL READ\n%s\n",strerror(errno));
		}
		if(use_exit_func){
			exit_func_for_this_module();
		}
	        return -1;
	}

}
return read_total;

}
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
			if (s < 0){
				return -1;
			}
			if (s == 0){
				return send_total;
			}
			}
			else if(!iResult){
					return -2;
			}
			else{
			if(logging){

			fprintf(logstream, "SELECT ERROR!!!!! SEND\n%s\n",strerror(errno));
			}
			if(use_exit_func){
				exit_func_for_this_module();
		    	}
			return -1;
			}
		}
		return send_total;
	}
	return -1;
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
				if(r < 0){
					return -1;
				}
				if(r == 0){
					return read_total;
				}
			}
			else if(!iResult){
	               	return -2;
			}
			else{
			if(logging){

			fprintf(logstream, "SELECT ERROR!!!!! READ\n%s\n",strerror(errno));
			}
			if(use_exit_func){
				exit_func_for_this_module();
		    	}
			return -1;
			}
			}
			return read_total;
		}
		return -1;
}

int sendallfd(int sock,int fd,int_pair times,uint8_t is_ssl,SSL* cSSL){

char buff[DEF_DATASIZE];
int numread;
int sent=0;
while ((numread = read(fd,buff,DEF_DATASIZE)) > 0) {
    int totalsent = 0;
    while (totalsent < numread) {
        errno=0;
	sent = is_ssl?sendsome_ssl(cSSL, buff + totalsent,  numread - totalsent,times):sendsome(sock, buff + totalsent,  numread - totalsent,times);
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
int readalltofd(int sock,int fd,size_t size,int_pair times,uint8_t is_ssl,SSL* cSSL){
    int32_t len=1;
	size_t total=0;
	char buff[DEF_DATASIZE];
	memset(buff,0,DEF_DATASIZE);
	if(is_ssl){
		for(;(len==-2||len>0)&&(total!=size);){
	                len=readsome_ssl(cSSL,buff,DEF_DATASIZE,times);
	                if(len > 0){
				size_t written_total = 0;
				while(written_total < (size_t)len){
					ssize_t w = write(fd, buff + written_total, len - written_total);
					if(w > 0){
						written_total += w;
						total += w;
					} else if(errno == EAGAIN || errno == EWOULDBLOCK){
						continue; // try again
					} else {
						if(logging){
							fprintf(logstream,"write error: %s\n", strerror(errno));
						}
						return -1;
					}
				}
			}
        	}
	}
	else{
		for(;(len==-2||len>0)&&(total!=size);){
	                len=readsome(sock,buff,DEF_DATASIZE,times);
	                if(len > 0){
				size_t written_total = 0;
				while(written_total < (size_t)len){
					ssize_t w = write(fd, buff + written_total, len - written_total);
					if(w > 0){
						written_total += w;
						total += w;
					} else if(errno == EAGAIN || errno == EWOULDBLOCK){
						continue; // try again
					} else {
						if(logging){
							fprintf(logstream,"write error: %s\n", strerror(errno));
						}
						return -1;
					}
				}
			}
	        }

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
