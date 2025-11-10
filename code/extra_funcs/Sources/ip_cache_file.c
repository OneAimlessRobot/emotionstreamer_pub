#include "../../Includes/preprocessor.h"
#include "../Includes/sockio.h"
#include "../Includes/sock_ops.h"
#include "../Includes/fileshit.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/ip_cache_file.h"

void print_ip_cache_entry(FILE* stream,ip_cache_entry* ent){

	fprintf(stream,PRINT_IP_CACHE_FILE,ent->hostname,ent->port);


}
void encode_ip_cache_entry(FILE* stream,ip_cache_entry* ent){

	fprintf(stream,ENCODE_IP_CACHE_FILE,ent->hostname,ent->port);


}
void parse_ip_cache_entry(char* str,ip_cache_entry* ent){

	sscanf(str,"%[^:]:%hu",ent->hostname,&ent->port);

}

int find_ip_addr_entry(ip_cache_entry* ent,ip_cache_t* cache){

	int result=-1;
	//printf("Addr cache comparison:\ntamanho de cache: %d\n",num_of_addr);
	for(int i=0;i<cache->num_of_addr;i++){
		int comp_result=memcmp(ent,&cache->ips[i],sizeof(ip_cache_entry));
		if(!comp_result){
			//printf("Comparacao devolveu igual!!!!\n");
			result=i;
			break;
		}
	}
	//printf("End of addr cache comaprison. Index: %d\n",result);

	return result;

}
int insert_ip_addr_entry(ip_cache_entry* ent,ip_cache_t* cache){

	int index=find_ip_addr_entry(ent,cache);
	print_ip_cache_entry(stdout,ent);
	//printf("Addr cache Insertion:\ntamanho de cache: %d\nEntry existe? %s\nIndex: %d\n",cache->num_of_addr,(index>=0) ? "SIM":"NAO",index);

	if(cache->num_of_addr>= PREV_ADDR_CACHE_MAX){
		//printf("Cache cheia! Nao ha inserçoes a fazer!!!!\n");
		return 0;
	}
	if(index>=0){
		//printf("Cache entry ja existe! Nao ha inserçoes a fazer!!!!\n");
		return 0;
	}
	memcpy(&cache->ips[cache->num_of_addr++],ent,sizeof(ip_cache_entry));
	//printf("End of addr cache insertion. Result: Inserçao feita!\nTamanho de cache: %d\n",cache->num_of_addr);
	print_ip_cache_entry(stdout,&(cache->ips[(cache->num_of_addr)-1]));
	print_ip_cache_entry(stdout,ent);
	return index;

}
