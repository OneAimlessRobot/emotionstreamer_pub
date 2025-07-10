#ifndef AUXFUNCS_H
#define AUXFUNCS_H
typedef enum var_op{V_LOOK,V_SET}var_op;

int acess_var_mtx(pthread_mutex_t* mtx,int* var,int value_if_change,var_op op);

//All strings are null terminated
int strs_are_strictly_equal(char* str1, char* str2);

int min(int first,int second);

int max(int first,int second);

void swap(void* first,void* second,int sz_bytes);

void print_addr_aux(char* prompt,struct sockaddr_in* addr);
int randInteger(int min, int max);
char* randStr(int size, char* buff);
void snprint_addr_aux(char* dst,uint32_t size,struct sockaddr_in* addr);
#endif
