#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sockio_tcp.h"
#include "../Includes/fileshit.h"


int64_t sendsome_ssl(SSL* ssl, const char* buf, int64_t len, int_pair times) {

	if(!ssl){
		return -1;
	}
	int sd= SSL_get_fd(ssl);
	if(sd < 0){
		return -1;
	}
	int64_t send_total = 0;
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set wrfds;
	FD_ZERO(&wrfds);
	FD_SET(sd, &wrfds);
	int iResult=select(sd + 1, (fd_set*)0, &wrfds, (fd_set*)0, &tv);
	if(iResult>0){
		send_total = SSL_write(ssl, buf, len);
	        if (send_total >= 0) {
	            return send_total;
	        }
		int ssl_err = SSL_get_error(ssl, send_total);
		if (ssl_err == SSL_ERROR_WANT_READ) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mrfds;
			FD_ZERO(&mrfds);
			FD_SET(sd, &mrfds);
			select(sd + 1, &mrfds, (fd_set*)0, (fd_set*)0, &mtv);
			return -2;
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mwfds;
			FD_ZERO(&mwfds);
			FD_SET(sd, &mwfds);
			select(sd + 1, (fd_set*)0, &mwfds, (fd_set*)0, &mtv);
			return -2;
		}
		else if (ssl_err == SSL_ERROR_ZERO_RETURN) {
			return send_total;
		}
		else if(ssl_err == SSL_ERROR_SYSCALL){
		    ERR_print_errors_fp(stderr);
		    if(logging){

				fprintf(logstream, "SSL SYSCALL ERROR AT SSL SEND\n%s\n",strerror(errno));
		    }
		    if(errno==EAGAIN){

			return -2;
		    }
		    if(errno == EWOULDBLOCK){
			return -2;
			}
		     else{
			    if(logging){

					fprintf(logstream, "Will emergency func be called? %s\n",use_exit_func?"Yes!":"No..:");
			    }
			    if((errno!=ECONNRESET)&&use_exit_func){
					exit_func_for_this_module();
			    }
			    return -1;
			}
		}
		else{
		    ERR_print_errors_fp(stderr);
		    if(errno==EAGAIN){

			return -2;
		    }
		    if(errno == EWOULDBLOCK){
			return -2;
		    }
		if(logging){

			fprintf(logstream, "SSL FATAL ERROR AT SSL SEND\n%s\nWill emergency func be called? %s\n",strerror(errno),use_exit_func?"Yes!":"No..:");
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

			fprintf(logstream, "SSL FATAL ERROR AT SSL SEND\n%s\nWill emergency func be called? %s\n",strerror(errno),use_exit_func?"Yes!":"No..:");
		}
		if(use_exit_func){
			exit_func_for_this_module();
		}
		return -1;
	}
	return send_total;


}

int64_t readsome_ssl(SSL* ssl, char* buf, int64_t len, int_pair times) {
	if(!ssl){
		return -1;
	}
	int sd= SSL_get_fd(ssl);
	if(sd < 0){
		return -1;
	}
	int has_pending_result=SSL_has_pending(ssl);
	int pending_result=SSL_pending(ssl);
	if(has_pending_result){
		if(pending_result > 0){
			goto read_label;
		}
	}
	int64_t read_total = 0;
	struct timeval tv;
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sd, &rfds);
	int iResult=select(sd + 1, &rfds, (fd_set*)0, (fd_set*)0, &tv);
	if(iResult>0){
		read_label:
		read_total = SSL_read(ssl, buf, len);
		if (read_total >= 0) {
			return read_total;
		}
		int ssl_err = SSL_get_error(ssl, read_total);
		if (ssl_err == SSL_ERROR_WANT_READ) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set mrfds;
			FD_ZERO(&mrfds);
			FD_SET(sd, &mrfds);
			select(sd + 1, &mrfds, (fd_set*)0, (fd_set*)0, &mtv);
			return -2;
		}
		else if (ssl_err == SSL_ERROR_WANT_WRITE) {
			struct timeval mtv;
			mtv.tv_sec=times[0];
			mtv.tv_usec=times[1];
			fd_set wfds;
			FD_ZERO(&wfds);
			FD_SET(sd, &wfds);
			select(sd + 1, (fd_set*)0, &wfds, (fd_set*)0, &mtv);
			return -2;
		}
		else if (ssl_err == SSL_ERROR_ZERO_RETURN) {
			return read_total;
		}
		else if(ssl_err == SSL_ERROR_SYSCALL){
		    ERR_print_errors_fp(stderr);
		    if(logging){

				fprintf(logstream, "SSL SYSCALL ERROR AT SSL READ\n%s\n",strerror(errno));
		    }
		    if(errno==EAGAIN){

			return -2;
		    }
		    if(errno == EWOULDBLOCK){
			return -2;
			}
		    else{
			    if(logging){

					fprintf(logstream, "Will emergency func be called? %s\n",use_exit_func?"Yes!":"No..:");
			    }
			    if((errno!=ECONNRESET)&&use_exit_func){
				    exit_func_for_this_module();
			    }
			    return -1;
			}
		    }
		else{
		    ERR_print_errors_fp(stderr);
		    if(errno==EAGAIN){

			return -2;
		    }
		    if(errno == EWOULDBLOCK){
			return -2;
		    }
		    if(logging){

			fprintf(logstream, "SSL FATAL ERROR AT SSL READ\n%s\nWill emergency func be called? %s\n",strerror(errno),use_exit_func?"Yes!":"No..:");
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

			fprintf(logstream, "SSL FATAL ERROR AT SSL READ\n%s\nWill emergency func be called? %s\n",strerror(errno),use_exit_func?"Yes!":"No..:");
		}
		if(use_exit_func){
			exit_func_for_this_module();
		}
	        return -1;
	}
	return read_total;

}
int64_t sendsome(int sd,char buff[],int64_t size,int_pair times, int flags){
	if(sd<0){
		return -1;
	}
	int iResult;
	struct timeval tv;
	int64_t send_total=0;
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(sd,&wfds);
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	iResult=select(sd+1,(fd_set*)0,&wfds,(fd_set*)0,&tv);
	if(iResult>0){
		send_total=send(sd,buff,size,flags);
		if (send_total  < 0){
			return -1;
		}
		return send_total;
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
	return send_total;
}

int64_t readsome(int sd,char buff[],int64_t size,int_pair times, int flags){
	if(sd<0){
		return -1;
	}
	int iResult;
	struct timeval tv;
	int64_t read_total=0;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sd,&rfds);
	tv.tv_sec=times[0];
	tv.tv_usec=times[1];
	iResult=select(sd+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
	if(iResult>0){
		read_total=recv(sd,buff,size,flags);
		if(read_total < 0){
			return -1;
		}
		return read_total;
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
	return read_total;
}

int64_t readall(int socket,SSL*ctx_if_ssl,char* buff, int64_t total_to_read, uint8_t is_tls, int_pair times){
	int64_t total=0,
		len=0;
	while(1){
		len=is_tls?readsome_ssl(ctx_if_ssl,buff+total, total_to_read-total,times):readsome(socket,buff+total,total_to_read-total, times ,0);
		if(len<0){
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				if(logging > 49){
					fprintf(logstream,"Would block/Try again later!!!!\n");
				}
				continue;
			}
			else if(errno==EPIPE){

				if(logging){
					fprintf(logstream,"Pipe partido!!!\n");
				}
				return -2;
			}
			else if(errno==ENOTCONN){
				if(logging){
					fprintf(logstream,"Li %ld ao todo!!!! readall saiu com erro!!!!!:\nAvisando para desconectar!\n%s\n",total,strerror(errno));
				}
				return -1;
			}
			else if(len!=-2){
				if(logging){
					fprintf(logstream,"Li %ld ao todo!!!! readall saiu com erro!!!!!:\n%s\n",total,strerror(errno));
				}
				return -1;
			}
		}
		else if(len >=0){
			total+=len;
			break;
		}
	}
	return total;

}
int64_t sendall(int socket,SSL*ctx_if_ssl,char* buff, int64_t total_to_send, uint8_t is_tls,int_pair times){
	int64_t total=0,
		len=0;
	while(1){
		len=is_tls?sendsome_ssl(ctx_if_ssl,buff+total, total_to_send-total, times):sendsome(socket,buff+total,total_to_send-total, times ,0);
		if(len<0){
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				if(logging > 49){
					fprintf(logstream,"Would block/Try again later!!!!\n");
				}
				continue;
			}
			else if(errno==EPIPE){

				if(logging){
					fprintf(logstream,"Pipe partido!!!\n");
				}
				return -2;
			}
			else if(errno==ENOTCONN){
				if(logging){
					fprintf(logstream,"Li %ld ao todo!!!! readall saiu com erro!!!!!:\nAvisando para desconectar!\n%s\n",total,strerror(errno));
				}
				return -1;
			}
			else if(len!=-2){
				if(logging){
					fprintf(logstream,"Li %ld ao todo!!!! readall saiu com erro!!!!!:\n%s\n",total,strerror(errno));
				}
				return -1;
			}
		}
		else if(len >=0){
			total+=len;
			break;
		}
	}
	return total;

}
int sendallfd(int sock,int fd,int_pair times,uint8_t is_ssl,SSL* cSSL){

char buff[DEF_DATASIZE];
int numread;
int sent=0;
while ((numread = read(fd,buff,DEF_DATASIZE)) > 0) {
    int totalsent = 0;
    while (totalsent < numread) {
        errno=0;
	sent = is_ssl?sendsome_ssl(cSSL, buff + totalsent,  numread - totalsent,times):sendsome(sock, buff + totalsent,  numread - totalsent,times,0);
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
int readalltofd(int sock,int fd,int64_t size,int_pair times,uint8_t is_ssl,SSL* cSSL){
    int32_t len=1;
	int64_t total=0;
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
	                len=readsome(sock,buff,DEF_DATASIZE,times,0);
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
