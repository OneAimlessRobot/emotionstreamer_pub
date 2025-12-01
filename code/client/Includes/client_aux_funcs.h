#ifndef CLIENT_AUX_FUNCS_H
#define CLIENT_AUX_FUNCS_H

typedef typeof(void *(void *)) thread_func;

pid_t gettid_here();

int set_this_thread_name(pid_t tid,const char * name);
int get_this_thread_name(pid_t tid,char name_storing_buff[DEF_DATASIZE]);

void create_client_thread(pthread_t* tid_ptr,thread_func func);

void join_client_thread(pthread_t t,char* thread_name_buff);

/*
the gate to my nightmares rn

I dont care.
I dont want to care.
I dont want care if the audio locks.

I dont [need] to care.


Fixing further can only widen this wound
I have been carving out of code for a year.

normal programs dont lock.
Chrome is not better that mitos.is.

And I dont need workarounds for problems that dont exist, okay?
Im better than a google developer.
Im better than ALL OF THEM!

I dont NEED HELP!
SO LEAVE ME ALONE!
LEAVE!

LEAVE ME ALONE!


Please...
just let me exist.

void join_client_thread_with_timeout(pthread_t t,char* thread_name_buff);

Im sorry if whole rant this sounds pathetic.

I promise I will just go back to programming like a good boy, okay?
I promise I will be good,
productive,
predictible and rational like everybody else.
I will not be cringe.
I will not roleplay my pain.
I will just program.
And if it hurts,
Ill just take some of my Risperidone 1mg and shut up about it like everyone else.
because.
If caring is what I am,
And caring is punished,
then I am no longer alive,
so Why bother understanding my pain?
Just swallow it down with pink candy.
it will be fine.

*/
#endif
