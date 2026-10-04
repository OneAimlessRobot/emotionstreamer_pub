#include "../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sock_ops.h"
#include "../Includes/more_socket_ops.h"
#include "../Includes/fileshit.h"

void set_sock_os_keepalive(int* socket, int on_or_off){

	if (setsockopt((*socket), SOL_SOCKET, SO_KEEPALIVE, &on_or_off, sizeof(on_or_off)) < 0) {
		perror("Error setting SO_KEEPALIVE");
		close(*socket);
		exit(-1);
	}


}

void set_tcp_socket_keep_idle(int * socket, int secs){

	if(setsockopt((*socket), IPPROTO_TCP, TCP_KEEPIDLE, &secs, sizeof(secs))<0){
                perror("Erro a meter TCP_KEEPALIVE (IPPROTO_TCP)  na socket (setsockopt)\n");
		close(*socket);
		exit(-1);
	}

}

void set_tcp_socket_keep_itvl(int* socket, int secs_period){

	if(setsockopt((*socket), IPPROTO_TCP, TCP_KEEPINTVL, &secs_period, sizeof(secs_period))<0){

		perror("Erro a meter TCP_KEEPINTVL (IPPROTO_TCP)  na socket (setsockopt)\n");
		close(*socket);
		exit(-1);

	}

}

void set_tcp_socket_keep_cnt(int* socket, int times){

	if(setsockopt((*socket), IPPROTO_TCP, TCP_KEEPCNT, &times, sizeof(times))<0){

		perror("Erro a meter TCP_KEEPCNT (IPPROTO_TCP)  na socket (setsockopt)\n");
		close(*socket);
		exit(-1);

	}


}



void create_safety_pipe(int safety_pipe[2],char* pipe_desc,char* module_desc,int flags){
if(pipe2(safety_pipe,flags)){

        dprintf(2,"Safety pipe named: %s on module named: %s failed to open! Aborting\nError: %s\n",pipe_desc,module_desc,strerror(errno));
        exit(-1);
}

if((safety_pipe[0]=dup(safety_pipe[0]))<0){

        dprintf(2,"Safety pipe named: %s on module named: %s failed to be duped on read end!!\nAborting\nError: %s\n",pipe_desc,module_desc,strerror(errno));
        exit(-1);


}
if((safety_pipe[1]=dup(safety_pipe[1]))<0){

        dprintf(2,"Safety pipe named: %s on module named: %s failed to be duped on read end!!\nAborting\nError: %s\n",pipe_desc,module_desc,strerror(errno));
        exit(-1);


}
printf("just created Safety pipe named: %s on module named: %s\n",pipe_desc,module_desc);

}




void set_sock_reuseaddr(int *socket,int on_off){
        int ptr=on_off;
        socklen_t sizeofbuff=sizeof(ptr);
        if(setsockopt(*socket,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeofbuff)){
                perror("Erro a meter SO_REUSEADDR  na socket (setsockopt)\n");
                close(*socket);
        	exit(-1);
        }
}

void set_sock_sendtimeout(int*socket,int timeout_s,int timeout_us){
        struct timeval tv={0};
        tv.tv_sec=timeout_s;
        tv.tv_usec=timeout_us;
        socklen_t sizeofbuff=sizeof(struct timeval);
        if(setsockopt(*socket,SOL_SOCKET,SO_SNDTIMEO,(char*)&tv,sizeofbuff)){
                perror("Erro a meter SO_SNDTIMEO  na socket (setsockopt)\n");
                close(*socket);
        exit(-1);
        }
}
void set_sock_recvtimeout(int*socket,int timeout_s,int timeout_us){
        struct timeval tv={0};
        tv.tv_sec=timeout_s;
        tv.tv_usec=timeout_us;
        socklen_t sizeofbuff=sizeof(struct timeval);
        if(setsockopt(*socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&tv,sizeofbuff)){
                perror("Erro a meter SO_RCVTIMEO  na socket (setsockopt)\n");
                close(*socket);
        exit(-1);
        }
}

void setSocketRecvBuffSize(int*sd,int size){

setsockopt(*sd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

}

void setSocketSendBuffSize(int*sd,int size){

setsockopt(*sd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)); // Send buffer 1K

}
int getSocketRecvBuffSize(int*sd){

int sizevaluecontainer=0;
socklen_t sizeofbuff=sizeof(sizevaluecontainer);
getsockopt(*sd, SOL_SOCKET, SO_RCVBUF, &sizevaluecontainer, &sizeofbuff);
return sizevaluecontainer;
}

int getSocketSendBuffSize(int*sd){

int sizevaluecontainer=0;
socklen_t sizeofbuff=sizeof(sizevaluecontainer);
getsockopt(*sd, SOL_SOCKET, SO_SNDBUF, &sizevaluecontainer, &sizeofbuff);
return sizevaluecontainer;
}

void setLinger(int*socket,int onoff,int time){

struct linger so_linger;
so_linger.l_onoff = onoff; // Enable linger option
so_linger.l_linger = time; // Linger time, set to 0

if (setsockopt(*socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)) < 0) {
    perror("setsockopt");
    // Handle error
}

}
void socket_close(int* fd, int right_now) {
    if (right_now) {
        setLinger(fd,1,0);
	shutdown(*fd, SHUT_RDWR);
    	close(*fd);
        return;
    }

    // Polite close (FIN)
    //shutdown(*fd, SHUT_RDWR);
    close(*fd);
}

int get_sockerr(int*sd){

	int sockerr=0;
	socklen_t socklen_here =sizeof(sockerr);
	getsockopt(*sd,SOL_SOCKET,SO_ERROR,(char*)&sockerr,&socklen_here);
	return sockerr;
}
int same_addr_sock_rebind(int*sd){

	int result=0;
	struct sockaddr_in sockaddr_for_rebind={0};
	getsockname(*sd, (struct sockaddr*)&sockaddr_for_rebind,&socklenvar[1]);
	socket_close(sd,1);
	(*sd)= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if((*sd)<0){
		result = -1;
        }
        set_sock_reuseaddr(sd,1);
        setNonBlocking(sd);
	if(bind(*sd,(struct sockaddr *)&sockaddr_for_rebind,socklenvar[1])){
                if(logging){
			perror("Não conseguimos dar re bind neste sockfd!!!\n");
                	print_addr_aux("Este é o address:",&sockaddr_for_rebind);
        	}
		result = 1;
	}
	else{

		if(logging){
			fprintf(logstream,"Rebind successful! trying again!\n");
		}
		result = 0;
	}
	return result;
}
