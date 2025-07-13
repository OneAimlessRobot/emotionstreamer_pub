#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../Includes/ip_cache_file_ops.h"

static int ip_addr_cache_fd=-1;
static FILE* ip_addr_cache_fp=NULL;

static void encode_ip_cache(FILE* stream,ip_cache_t* encoded_cache){

        printf("Prev addr cache:\n");
        for(int i=0;i<encoded_cache->num_of_addr;i++){

                encode_ip_cache_entry(stream,&encoded_cache->ips[i]);

        }
        printf("End of prev addr cache:\n");

}
void print_ip_cache(FILE* stream,ip_cache_t* printed_cache){

        printf("Prev addr cache:\n");
        printf("Numero de addresses presentes: %d\n",printed_cache->num_of_addr);
	for(int i=0;i<printed_cache->num_of_addr;i++){

                print_ip_cache_entry(stream,&printed_cache->ips[i]);

        }
        printf("End of prev addr cache:\n");

}

static void open_ip_addr_cache_fp(char* flags){

        if(!(ip_addr_cache_fp=fopen(PREV_ADDR_FILE_NAME_CLIENT, flags))){

                perror("Nao foi possivel abrir cache de ips!!!!!!\n");
                raise(SIGINT);

        }


}


static int open_ip_cache_file(void){
        int result=0;
        if((ip_addr_cache_fd=open(PREV_ADDR_FILE_NAME_CLIENT,O_RDONLY,0777))<0){
                result=1;
                perror("Ficheiro de cache de ips nao existe. criando!!!!!!!!!\n");


        }
        close(ip_addr_cache_fd);
        if((ip_addr_cache_fd=open(PREV_ADDR_FILE_NAME_CLIENT,O_CREAT|O_RDONLY,0777))<0){
                result=2;
                perror("Ficheiro de cache de ips nao existe. Tentamos criar e n deu!!!!!!!!!\n");

        }
        if(result){

                raise(SIGINT);
        }

        open_ip_addr_cache_fp("r");

	return result;


}


static void populate_ip_addr_cache(ip_cache_t* con_cache){

        open_ip_cache_file();


        char buff[PATHSIZE*2]={0};
        while(fgets(buff, PATHSIZE*2, ip_addr_cache_fp)){

                con_cache->num_of_addr++;
                memset(buff,0,PATHSIZE*2);
        }
        min(PREV_ADDR_CACHE_MAX,con_cache->num_of_addr);
        fclose(ip_addr_cache_fp);
        ip_addr_cache_fp=NULL;

        memset(con_cache->ips,0,sizeof(ip_cache_entry)*PREV_ADDR_CACHE_MAX);
        memset(buff,0,PATHSIZE*2);

        open_ip_addr_cache_fp("r");
        int curr=0;
        while(fgets(buff, PATHSIZE*2, ip_addr_cache_fp)){

                parse_ip_cache_entry(buff,&con_cache->ips[curr++]);
                memset(buff,0,PATHSIZE*2);
        }



}





void init_ip_addr_cache(ip_cache_t* con_cache,ip_cache_entry* buff){
        con_cache->num_of_addr=0;
	con_cache->ips=buff;
        populate_ip_addr_cache(con_cache);
        fclose(ip_addr_cache_fp);
}

void save_ip_addr_entry_cache(ip_cache_t* saved_cache){

        if((ip_addr_cache_fd=open(PREV_ADDR_FILE_NAME_CLIENT,O_WRONLY|O_TRUNC,0777))<0){
                perror("Nao foi possivel abrir ficheiro para truncar!!!! Saindo!!!\n");
                raise(SIGINT);

        }
        open_ip_addr_cache_fp("w");
        encode_ip_cache(ip_addr_cache_fp,saved_cache);
        fclose(ip_addr_cache_fp);

}
int try_cache_connect(int* sock,int_pair times_pair,ip_cache_t* con_cache){
	
	int result=0;
	struct sockaddr_in curr_addr;
        for(int i=0;i<con_cache->num_of_addr;i++){
		(*sock)= socket(AF_INET,SOCK_STREAM,0);

		if(*sock<0){
			
			raise(SIGINT);
		}
		init_addr(&curr_addr,(con_cache->ips[i]).hostname,(con_cache->ips[i]).port);
                printf("Index: %d\n",i);
		print_addr_aux("",&curr_addr);
		result=tryConnect(sock,times_pair,&curr_addr);
		print_ip_cache_entry(stdout,&(con_cache->ips[i]));
                if(result){
                        printf(CACHE_CONNECT_MADE);
                        return result;
                }
		close(*sock);
        }
        perror(NO_CACHE_CONNECT_MADE);
	return result;

}

