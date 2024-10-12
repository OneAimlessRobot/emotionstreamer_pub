#include "../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/protocol.h"
#include "../Includes/client.h"

static struct sockaddr_in server_address;
long int dataSize;
static int client_socket;
static int fd;



static void tryConnect(int* socket){
        int success=-1;
        int numOfTries=MAX_TRIES;


        while(success==-1&& numOfTries){
                printf(CONNECTION_ATTEMPT_NO_MSG,inet_ntoa(server_address.sin_addr),-numOfTries+MAX_TRIES+1);
                success=connect(*socket,(struct sockaddr*)&server_address,sizeof(server_address));
               int sockerr;
                socklen_t slen=sizeof(sockerr);
                getsockopt(*socket,SOL_SOCKET,SO_ERROR,(char*)&sockerr,&slen);
                numOfTries--;
                if(sockerr==EINPROGRESS){

                        fprintf(stderr,SOCK_ERROR_DUMP,strerror(errno),strerror(sockerr),*socket);
                        continue;

                }
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET((*socket),&wfds);

                struct timeval t;
                t.tv_sec=CLIENT_TIMEOUT_CON_SEC;
                t.tv_usec=CLIENT_TIMEOUT_CON_USEC;
                int iResult=select((*socket)+1,0,&wfds,0,&t);

                if(iResult>0&&!success&&((*socket)!=-1)){
			break;

                }
                fprintf(stderr,"Não foi possivel: %s\n",strerror(errno));
        }
        if(!numOfTries){
        printf(MAX_TRIES_HIT_ERROR,MAX_TRIES);
        raise(SIGINT);
        }

}


static void sigpipe_handler(int signal){

	close(client_socket);
	close(fd);
	printf("%s\n",strerror(errno));
	exit(-1);
}
static void sigint_handler(int signal){

	close(client_socket);
	close(fd);
	printf("cliente a fechar!!!\n");
	exit(-1);

}

//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,u_int64_t port, char* host){
	
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigint_handler);
	
	logstream=stderr;
	logging=1;
	
	req_type the_type= str_to_req_type(req_field);
	//receber e armazenar dados recebidos
	char buff[DEF_DATASIZE]={0};
	char file_name_container[PATHSIZE]={0};
	
	char req_str_res[BUFFSIZE]={0};

	char file_path[PATHSIZE]={0};
	
	
	req_type_to_str(the_type,req_str_res);
	
	snprintf(file_path,PATHSIZE-1,"%s%s",curr_dir,file_name);
	
	if(the_type==NA){
		printf(UNKNOWN_REQ);
		raise(SIGINT);
	}


	client_socket= socket(AF_INET,SOCK_STREAM,0);
	if(client_socket==-1){
		raise(SIGINT);
	}
	/*
       long flags= fcntl(client_socket,F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(client_socket,F_SETFD,flags);
	*/
	server_address.sin_family=AF_INET;
	server_address.sin_port= htons(port);
	struct hostent* hp= gethostbyname(host);
	//server_address.sin_addr.s_addr = inet_addr(argv[3]);	
	memcpy(&(server_address.sin_addr),hp->h_addr,hp->h_length);
	tryConnect(&client_socket);
	printf(CONNECTION_ESTABLISHED_MSG,inet_ntoa(server_address.sin_addr));
	
	
        //enviar a string de request
        sendsome(client_socket,req_str_res,BUFFSIZE,CLIENT_DATA_TIMES_PAIR);

        //Ler a resposta do server feita pq se alterna sempre reads com writes e quis enviar o request e o filename em separado
        readsome(client_socket,req_str_res,BUFFSIZE,CLIENT_DATA_TIMES_PAIR);

        snprintf(file_name_container,PATHSIZE-1,"%s",file_name);
        //Enviando o filename
        sendsome(client_socket,file_name_container,PATHSIZE-1,CLIENT_DATA_TIMES_PAIR);





        //lendo o datasize
        readsome(client_socket,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR);
        printf("Data size buff: %s\n",buff);
        //Dizendo ao server "Hey! Recebemos datasize!!!! O q enviamos n é importante. So tem de ser "DEF_DATASIZE" Bytes em dimensão
        sendsome(client_socket,buff,DEF_DATASIZE,CLIENT_DATA_TIMES_PAIR);
        sscanf(buff,"%ld",&dataSize);
        printf(DOWNLOAD_SIZE_MSG,dataSize);
        if(!dataSize){
                printf(NO_SUCH_FILE_IN_SERVER);
                raise(SIGINT);
        }


	//especificar socket;
	
	switch(the_type){

	case GET:
		if((fd= creat(file_path,0777))<0){
			perror(FILE_CREATION_ERROR);
			raise(SIGINT);
		}
		else{
			printf(CREATED_FILE_PATH_MSG, file_path);
		}
		break;
	case PEEK:
		fd=1;
		printf(CONTENT_PEEK_INCOMMING);
		break;
	default:
		break;
	}

	readalltofd(client_socket,fd,CLIENT_DATA_TIMES_PAIR);
	
	if(fd>2){
		close(fd);
		//player_init(file_path);
	}
	return 0;
}

