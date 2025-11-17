#include "../../Includes/preprocessor.h"
#include "../Includes/sockio.h"
#include "../Includes/ip_cache_file.h"
#include "../Includes/connection.h"
#include "../Includes/sock_ops.h"
#include "../Includes/fileshit.h"
#include "../Includes/auxfuncs.h"


void setNonBlocking(int*socket) {
    int flags = fcntl(*socket, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr,"erro a atribuir flags a uma socket de cliente: %s\n",strerror(errno));
	return;
    }

    if (fcntl(*socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        fprintf(stderr,"erro a atribuir flags a uma socket de cliente: %s\n",strerror(errno));
    }
}



void print_sock_addr(int socket){

	struct sockaddr_in addr={0};

	getsockname(socket,(struct sockaddr*)(&addr),socklenvar);

	print_addr_aux("O endereço desta socket é:\n",&addr);

}
int init_addr(struct sockaddr_in* addr, char* hostname_str,uint16_t port){

        addr->sin_family=AF_INET;
	struct addrinfo *addr_info_struct=NULL;
	int error=0;
        if((error=getaddrinfo(hostname_str, NULL, NULL, &addr_info_struct))){
		if(logging){
			printf("Erro a obter address a partir de hostname!!\nErro: %s\n",gai_strerror(error));
		}
		if(addr_info_struct){
			freeaddrinfo(addr_info_struct);
			if(logging){
				fprintf(logstream,"Demos free a um addr info!!!\n");
			}
		}
		return 1;
	}
	memcpy(addr,(struct sockaddr_in*)addr_info_struct->ai_addr,sizeof(struct sockaddr_in));

	addr->sin_port= htons(port);
	if(logging){
		print_addr_aux("ip address: ",addr);
	}
	if(addr_info_struct){
		freeaddrinfo(addr_info_struct);
		if(logging){
			fprintf(logstream,"Demos free a um addr info!!!\n");
		}
	}
	return 0;
}

int tryConnect(int*socket,int_pair times_pair,struct sockaddr_in* dst_addr){
        int success=-1;
        int numOfTries=MAX_TRIES;
	int already_in_progress=0;


        while(success==-1&& numOfTries){
                if(!already_in_progress){
			if(logging){
				print_addr_aux("Tentando conectar a:",dst_addr);
				fprintf(logstream,"(Tentativa %d)\n",-numOfTries+MAX_TRIES+1);
	                }
			success=connect(*socket,(struct sockaddr*)dst_addr,sizeof(struct sockaddr));
	        	numOfTries--;
		}
	        int sockerr=0;
		socklen_t socklen_here =sizeof(sockerr);
	        getsockopt(*socket,SOL_SOCKET,SO_ERROR,(char*)&sockerr,&socklen_here);
	        if(logging){
			fprintf(logstream,"Erro normal:%s\n Erro Socket: %s\nNumero socket: %d\n",strerror(errno),strerror(sockerr),*socket);
		}
		if(logging){
			fprintf(logstream,"Already in progress!\n");
		}
		fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(*socket,&wfds);

                struct timeval t;
                t.tv_sec=times_pair[0];
                t.tv_usec=times_pair[1];
                int iResult=select((*socket)+1,0,&wfds,0,&t);
		if(iResult>0&&!success){
			if(logging){
				print_addr_aux("Conexao de:",dst_addr);
                        }
			break;

                }
		else if(iResult<=0){
			if(iResult){
				if(logging){
					fprintf(logstream,"Select error!!\n");
				}
				numOfTries=0;
				break;
			}
			else{

				if(logging){
					fprintf(logstream,"Select timeout reached!!\n");
				}
				continue;
			}
		}
		if(errno==ECONNREFUSED){

			numOfTries=0;
			break;
		}
                if(errno==ENOTSOCK){
			if(logging){
				fprintf(logstream,"Not a socket!!!\n");
			}
			numOfTries=0;
			break;
		}
                if(logging){
			fprintf(logstream,"Não foi possivel: %s\n",strerror(errno));
        	}
		if(errno==EALREADY){
			if(!already_in_progress){
				already_in_progress=1;
				numOfTries--;
			}
			continue;
		}
		else{
			already_in_progress=0;
		}
		if((errno == EINPROGRESS)){

			continue;
		}
	}
        if(!numOfTries){
		if(logging){
        		fprintf(logstream,"Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAX_TRIES);
        	}
	}

	return numOfTries-(((errno == EINPROGRESS)&&!numOfTries)?1:0);
}
