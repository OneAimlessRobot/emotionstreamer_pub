#include "../../Includes/preprocessor.h"
#include "../Includes/sockio.h"
#include "../Includes/ip_cache_file.h"
#include "../Includes/connection.h"
#include "../Includes/sock_ops.h"
#include "../Includes/more_socket_ops.h"
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
			printf("Erro a obter address a partir de hostname!!\nHostname tendado: |%s|\nErro: %s\n",hostname_str?hostname_str:"(null)",gai_strerror(error));
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

int tryConnect(int*sockfd,int_pair times_pair,struct sockaddr_in* dst_addr){
	int success=-1;
	uint16_t port=0;
	int numOfTries=MAX_TRIES;
	char addr_buff[DEF_DATASIZE]={0};
	while(success==-1&& numOfTries){
		if(logging){
			print_addr_aux("Tentando conectar a:",dst_addr);
			fprintf(stdout,"(Tentativa %d)\n",-numOfTries+MAX_TRIES+1);
		}
		success=connect(*sockfd,(struct sockaddr*)dst_addr,sizeof(struct sockaddr));
		numOfTries--;
		if(success<0){
			if(!(errno == EINPROGRESS)){
				if(logging){
					fprintf(stderr,"Não foi possivel: %s\n",strerror(errno));
				}
				numOfTries=0;
				break;
			}
			else{
				fd_set wfds;
				FD_ZERO(&wfds);
				FD_SET(*sockfd,&wfds);
				struct timeval t={times_pair[0],times_pair[1]};
				int iResult=select((*sockfd)+1,0,&wfds,0,&t);
				if(iResult>0){
					int sockerr=get_sockerr(sockfd);
					if(!sockerr){
						if(logging){
							fprintf(stderr,"Successful connection!\n");
							print_addr_aux("Conectado a:",dst_addr);
						}
						break;
					}
				}
				else{
					if(iResult){
						if(errno==EINTR){
							numOfTries=0;
							break;
						}
						if(logging){
							fprintf(stderr,"Select error!! %s\n",strerror(errno));
						}
						numOfTries=0;
						break;
					}
					else{
						if(logging){
							fprintf(stderr,"Select timeout reached!!\n");
						}
						if(same_addr_sock_rebind(sockfd)){
							numOfTries=0;
							break;
						}
						continue;
					}
				}
			}
		}
		else{
			snprint_addr_aux(addr_buff,&port,sizeof(addr_buff),dst_addr);
			if(logging){
				fprintf(stderr,"%s\n%s\n",errno?strerror(errno):"Successful connection",errno?"Não conectado.":addr_buff);
			}
			numOfTries=(errno?0:numOfTries);
			break;
		}
	}
	if(!numOfTries){
		if(logging){
			fprintf(stderr,"Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAX_TRIES);
		}
	}
	return numOfTries-(((errno == EINPROGRESS)&&!numOfTries)?1:0);
}
