#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"

int acess_var_mtx(pthread_mutex_t* mtx,int* var,int value_if_change,var_op op){


          int result=0;
          pthread_mutex_lock(mtx);
          switch(op){
                  case V_LOOK:
                  result=(*var);
                  break;
                  case V_SET:
                  (*var)=value_if_change;
                  break;
                  default:
                  break;
          }
          pthread_mutex_unlock(mtx);
          return result;



}

int strs_are_strictly_equal(char* str1, char* str2){

	int max_size= max(strlen(str1),strlen(str2));
	if((strnlen(str1,max_size)-strnlen(str2,max_size))){

		return -2;

	}


	return strncmp(str1,str2,max_size);


}

int min(int first,int second){


	if(first<second){


		return first;
	}

	else{

		return second;
	}


}

void swap(void* first,void* second,int sz_bytes){

	void* tmp = malloc(sz_bytes);

	memcpy(tmp,first,sz_bytes);

	memcpy(first,second,sz_bytes);

	memcpy(second,tmp,sz_bytes);

}

int max(int first,int second){


	if(first>second){


		return first;
	}

	else{

		return second;
	}


}
void print_addr_aux(char* prompt,struct sockaddr_in* addr){
         printf("%s\nEndereço: \n%s Porta: %u\n",prompt,inet_ntoa(addr->sin_addr),ntohs(addr->sin_port));

}
void snprint_addr_aux(char* dst,uint32_t size,struct sockaddr_in* addr){
         snprintf(dst,size-1,"%s",inet_ntoa(addr->sin_addr));

}
int randInteger(int min, int max) {
    // Seed the random number generator with the current time.


    // Generate a random number between 0 and RAND_MAX.
    int random = rand();

    // Scale the random number to fit within the specified range.
    int result = (random % (max - min + 1)) + min;

    return result;
}


//NULL TERMINATED!!
char* randStr(int size,char*buff){
	
        struct timespec time;

        clock_gettime(CLOCK_REALTIME, &time);
        srand(time.tv_nsec);

	
        memset(buff,0,size);
        for(int i=0;i<=size-1;i++){

                buff[i]=(char)randInteger((int)67,(int)91);

        }
        return buff;



}
