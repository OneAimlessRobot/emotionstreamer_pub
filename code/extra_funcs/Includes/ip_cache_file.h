#ifndef IP_CACHE_FILE_H
#define IP_CACHE_FILE_H

#define PRINT_IP_CACHE_FILE "Entry de address <ip>:<port>  %s:%hu\n"
#define ENCODE_IP_CACHE_FILE "%s:%hu\n"
#define CACHE_CONNECT_MADE "Conexão por cache!!!!!\n"
#define PRINT_IP_CACHE_FILE "Entry de address <ip>:<port>  %s:%hu\n"
#define PREV_ADDR_CACHE_MAX 80

typedef struct ip_cache_entry{


	char hostname[PATHSIZE];
	uint16_t port;


}ip_cache_entry;

typedef struct ip_cache_t{

	ip_cache_entry * ips;
	int num_of_addr;


}ip_cache_t;


//num_addr e o numero absoluto de endereços contidos em buff!!!!!
void print_ip_cache_entry(FILE* stream,ip_cache_entry* ent);
void encode_ip_cache_entry(FILE* stream,ip_cache_entry* ent);
int insert_ip_addr_entry(ip_cache_entry* ent,ip_cache_t* cache);
int find_ip_addr_entry(ip_cache_entry* ent,ip_cache_t* cache);
void parse_ip_cache_entry(char* str,ip_cache_entry* ent);







#endif
