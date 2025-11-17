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

int tryConnect(int*sockfd,int_pair times_pair,struct sockaddr_in* dst_addr){
        int success=-1;
        int numOfTries=MAX_TRIES;

        while(success==-1&& numOfTries){
                if(logging){
			print_addr_aux("Tentando conectar a:",dst_addr);
			fprintf(logstream,"(Tentativa %d)\n",-numOfTries+MAX_TRIES+1);
                }
		success=connect(*sockfd,(struct sockaddr*)dst_addr,sizeof(struct sockaddr));
        	numOfTries--;
	        int sockerr=0;
		socklen_t socklen_here =sizeof(sockerr);
	        getsockopt(*sockfd,SOL_SOCKET,SO_ERROR,(char*)&sockerr,&socklen_here);
	        if(logging){
			fprintf(logstream,"Erro normal:%s\n Erro Socket: %s\nNumero socket: %d\n",strerror(errno),strerror(sockerr),*sockfd);
		}
		if((success==-1)&&(errno == EINPROGRESS)){
		fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(*sockfd,&wfds);

                struct timeval t;
                t.tv_sec=times_pair[0];
                t.tv_usec=times_pair[1];
                int iResult=select((*sockfd)+1,0,&wfds,0,&t);
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
			}
		}
		}	
		else if(errno==ECONNREFUSED){

			numOfTries=0;
			break;
		}
        else if(errno==ENOTSOCK){
			if(logging){
				fprintf(logstream,"Not a socket!!!\n");
			}
			numOfTries=0;
			break;
		}
		else{
			if(logging){
			fprintf(logstream,"Não foi possivel: %s\n",strerror(errno));
        	}
			struct sockaddr_in sockaddr_for_rebind={0};
			getsockname(*sockfd, (struct sockaddr*)&sockaddr_for_rebind,&socklenvar[1]);
			socket_close(sockfd,1);
			(*sockfd)= socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		        if((*sockfd)<0){
				numOfTries=0;
				break;
		        }
		        set_sock_reuseaddr(sockfd,1);
		        setNonBlocking(sockfd);
			if(bind(*sockfd,(struct sockaddr *)&sockaddr_for_rebind,socklenvar[1])){
		                perror("Não conseguimos dar re bind neste sockfd!!!\n");
		                print_addr_aux("Este é o address:",&sockaddr_for_rebind);
		        	numOfTries=0;
				break;
			}
			else{

				if(logging){
					fprintf(logstream,"Rebind successful! trying again!\n");
				}
				numOfTries++;
			}
		}
	}
        if(!numOfTries){
		if(logging){
        		fprintf(logstream,"Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAX_TRIES);
        	}
	}

	return numOfTries-(((errno == EINPROGRESS)&&!numOfTries)?1:0);
}
