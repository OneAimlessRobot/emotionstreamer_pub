#ifndef IP_CACHE_FILE_OPS_H
#define IP_CACHE_FILE_OPS_H

#define MAX_ADDR_CACHE_NUM_MSG "Numero maximo de addresses atingido!!!!\n"
#define NO_IP_CACHE_OPENED "Nao foi possivel abrir cache de ips!!!!!!\n"
#define NO_CACHE_CONNECT_MADE "Nenhuma das entries da cache serviu!!!! Abortando conexão por cache!!!!!\n"
#define CACHE_CONNECT_MADE "Conexão por cache!!!!!\n"
#define PREV_ADDR_FILE_NAME_CLIENT "./.prev.addr"
#define PREV_ADDR_STRING "prev_addr"

void save_ip_addr_entry_cache(ip_cache_t*saved_cache);
int try_cache_connect(int* socket,int_pair times_pair,ip_cache_t* con_cache);
void init_ip_addr_cache(ip_cache_t* con_cache,ip_cache_entry* buff);
void print_ip_cache(FILE* stream,ip_cache_t* printed_cache);

#endif
