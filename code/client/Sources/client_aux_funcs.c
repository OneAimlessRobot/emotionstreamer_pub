#define _GNU_SOURCE
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
/*
I needed help.
I needed help again.
I needed help again.
I had no idea pthread_timedjoin_np existed.
let alone that It had to take an actual timestamp
and not an interval as an argument for when it should give up.
Chat gpt had to write this for me.
I just told it I had an issue.
But it found a solution.
And now I dont know what to do.
Am I fake?
Is all that ive done fake?
it was the same thing with the execvp implementation.
Why is there a solution for everything?
Why can my program not just exist in peace?
Why does it feel so,
Gross?
Why do I feel so disgusted at myself?
Why?
Why?
Why?
Why?
Why?
Please.
Someone free me from this nightmare.
Ive been refining this stupid streamer for a year
Im obsessed.
Im tired.
Im spent.
More solutions keep coming.
I just want to be fine with an imperfect program.
I rather leave it imperfect
than leave it other.
It needs to be mine to exist.
Why can I never fix things while keeping them mine?
Why am I so useless?
why did I need chatgpt to help me write the frame parser for the boundary files?
Why can I not even make the most important parts of my program feel mine?
Why can nothing be mine?
Why does everything have to belong to other people?
Why am I so bad?
Why am I always last place?
Why am I so fucking useless?
why does this program not feel mine anymore?
Why does the frustration mount so much with every tip from this stupid chat bot?
Why can I never create anything on my own?
Why?
Why?
Why?
Why?
Why does every problem I choose to ignore feel like cope,
and why does the solution to every problem I have feel like erasure?
I have a problem.
I dont know how to solve it.
Chatbots,
which turn anything you make into invalid slob,
are the ones who have the solution.
I could not find it anywhere with google.
I mean,
maybe someday I would have found timedjoin...
but probably...
never this fast.
Never.
Never this expeditiously.

I have not ran the code, by the way.
I have no idea if it even works.
But the fact that this thing is the only thing that will help me prevent the playing thread
from blocking when trying to join it,
And the fact it came from a chatbot,

and the fact...
that it is helpful,
outside of my influence.
outside of my merit.
merit to own something made by me,
that everyone can use without hickups.



It makes me want to die.
it makes me genuinely want to curl up in a ball and never leave my gross ass bedroom ever again.
Throw away every single piece of food in my house
sleep in the bathroom

defecate on the floor on which I sob,
wrapped in myself like a piece of dogcrap after hardening to the air.

FUCK me, im USELESS.
*/

static int join_with_timeout(pthread_t t, int seconds)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += seconds;

    int rc = pthread_timedjoin_np(t, NULL, &ts);

    if (rc == 0) return 1;        // joined
    if (rc == ETIMEDOUT) return 0; // hung
    return -1;                   // error
}

void join_client_thread_with_timeout(pthread_t t,char* thread_name_buff){
        printf("Saimos de \"%s\"?\nCom %d segundos de timeout?\n",thread_name_buff,9);
        if(!join_with_timeout(t,9)){

		printf("Não...\nTivemos de dar detach a \"%s\" para sair do programa\n",thread_name_buff);
		pthread_detach(t);
	}
	else{
        	printf("Sim!\nSaimos de \"%s\"!!!!!!\n",thread_name_buff);
	}
}

