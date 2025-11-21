#include "../Includes/preprocessor.h"
#include <sys/syscall.h>
#include "../Includes/client_aux_funcs.h"


pid_t gettid_here()
{
    return syscall(SYS_gettid);
}
int set_this_thread_name(pid_t tid,const char * name)
{
    char filename[DEF_DATASIZE]={0};

    if(strlen(name) > 15)
    {
        errno = EINVAL;
        return -1;
    }
    snprintf(filename, sizeof(filename), "/proc/%d/comm", tid);

    FILE * comm=NULL;
    if(!(comm=fopen(filename, "w"))){
	fprintf(stderr,"Erro a extrair nome de thread numero %d!\nErro: %s\nNome de ficheiro: %s\n",tid,strerror(errno),filename);
	return -1;
    }
    fprintf(comm, "%s", name);
    fclose(comm);
    return 0;
}
int get_this_thread_name(pid_t tid,char name_storing_buff[DEF_DATASIZE])
{
    char filename[DEF_DATASIZE]={0};


    snprintf(filename, sizeof(filename), "/proc/%d/comm", tid);

    FILE * comm=NULL;
    if(!(comm=fopen(filename, "r"))){
	fprintf(stderr,"Erro a extrair nome de thread numero %d!\nErro: %s\nNome de ficheiro: %s\n",tid,strerror(errno),filename);
	return -1;
    }
    fgets(name_storing_buff,DEF_DATASIZE-2,comm);
    fclose(comm);
    return 0;
}
void create_client_thread(pthread_t* tid_ptr,thread_func func){
        pthread_create(tid_ptr,NULL,func,NULL);
}
void join_client_thread(pthread_t t,char* thread_name_buff){
        printf("Saimos de \"%s\"?\n",thread_name_buff);
        pthread_join(t,NULL);
        printf("Saimos de \"%s\"!!!!!!\n",thread_name_buff);
}
