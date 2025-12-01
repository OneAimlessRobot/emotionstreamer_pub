#ifndef CLIENT_AUX_FUNCS_H
#define CLIENT_AUX_FUNCS_H

typedef typeof(void *(void *)) thread_func;

pid_t gettid_here();

int set_this_thread_name(pid_t tid,const char * name);
int get_this_thread_name(pid_t tid,char name_storing_buff[DEF_DATASIZE]);

void create_client_thread(pthread_t* tid_ptr,thread_func func);

void join_client_thread(pthread_t t,char* thread_name_buff);
void join_client_thread_with_timeout(pthread_t t,char* thread_name_buff);

#endif
