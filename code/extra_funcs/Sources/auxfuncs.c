#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include <dirent.h>

void print_out_logo(void){
	FILE* logo_fp=NULL;
	char line_buff[DEF_DATASIZE];
	if(!(logo_fp=fopen(LOGO_ASCII_ART_FILE_PATH,"r"))){

		fprintf(stderr,"Could not display logo!\nError: %s\n",strerror(errno));
		return;
	}
        printf(ANSI_BACKGROUND_WHITE ANSI_COLOR_BLUE ANSI_STYLE_BOLD);
	while(1){


		memset(line_buff,0,sizeof(line_buff));
		char* result=fgets(line_buff,sizeof(line_buff)-1,logo_fp);
                if(result){
			line_buff[strlen(line_buff)-1]=0;
			usleep(MS_TO_US(LOGO_ASCII_ART_FILE_SLEEP_PRINT_TIME_MS));
	        	printf("%s\n",line_buff);
		}
		else{
			if(feof(logo_fp)){

				fprintf(stdout,"We reached the end of the logo file and stopped printing!\n");
			}
			else{
				fprintf(stderr,"We stopped printing due to some error!!!!\nError: %s\n",strerror(errno));
			}
			fclose(logo_fp);
			logo_fp=NULL;
			break;
		}
	}
	printf(ANSI_RESET_ALL ANSI_BACKGROUND_BLACK ANSI_COLOR_WHITE);



}
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

int does_dir_exist_aux(char* path){

	DIR* directory=opendir(path);
	if(directory){
		closedir(directory);
	}
	errno=0;
	return (directory!=NULL);
}

uint16_t acess_var_mtx_uint16(pthread_mutex_t* mtx,uint16_t* var,uint16_t value_if_change,var_op op){


          uint16_t result=0;
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
void snprint_addr_aux(char* dst,uint16_t* port,uint32_t size,struct sockaddr_in* addr){
         snprintf(dst,size-1,"%s",inet_ntoa(addr->sin_addr));
	(*port)=addr->sin_port;
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
int proto_is_tcp(int proto){
	return proto<=0;

}
//https://stackoverflow.com/questions/1442116/how-can-i-get-the-date-and-time-values-in-a-c-program
void print_current_date(void){

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

}

void time_spec_sum_function(struct timeval* time_one,struct timeval* time_two,struct timeval* time_out){

	long int total_microsecs_one = S_TO_US(time_one->tv_sec)+time_one->tv_usec;
	long int total_microsecs_two = S_TO_US(time_two->tv_sec)+time_two->tv_usec;


	long int total_microsecs_out= total_microsecs_one+total_microsecs_two;

	long int partial_secs_out= US_TO_S(total_microsecs_out);
	long int partial_usecs_out= total_microsecs_out-(S_TO_US(partial_secs_out));

	time_out->tv_sec=partial_secs_out;
	time_out->tv_usec=partial_usecs_out;


}

//we assume time two is bigger!!!
//it is two  minus one!
//order matters!
void time_spec_sub_function(struct timeval* time_one,struct timeval* time_two,struct timeval* time_out){

	long int total_microsecs_one = S_TO_US(time_one->tv_sec)+time_one->tv_usec;
	long int total_microsecs_two = S_TO_US(time_two->tv_sec)+time_two->tv_usec;


	long int total_microsecs_out= total_microsecs_two-total_microsecs_one;

	long int partial_secs_out= US_TO_S(total_microsecs_out);
	long int partial_usecs_out= total_microsecs_out-(S_TO_US(partial_secs_out));

	time_out->tv_sec=partial_secs_out;
	time_out->tv_usec=partial_usecs_out;

}


void time_spec_print_function(int fd, const char* timeval_name, struct timeval* time_printed){

	dprintf(fd, "timeval_name: %s\n"
			"Printed timeval:\n"
			"time_printed->tv_sec: %ld secs\n"
			"time_printed->tv_usec: %ld usecs\n",
			timeval_name,
			time_printed->tv_sec,
			time_printed->tv_usec);
}

//time_one bigger (1), smaller(-1) or equal (0) to time_two?
int time_spec_compare_function(struct timeval* time_one,struct timeval* time_two){
	int secs_sub=(time_one->tv_sec-time_two->tv_sec);
	if(secs_sub>0){
		return 1;
	}
	else if(secs_sub){

		return -1;
	}
	else {
		int microsecs_sub=(time_one->tv_usec-time_two->tv_usec);
		if(microsecs_sub>0){
			return 1;
		}
		else if(microsecs_sub){

			return -1;
		}
		else{
			//they are the one and the same.
			//just not by reference necessarily of course
			return 0;
		}

	}
	//default return value for warning removal is 0
	return 0;
}



void skip_config_comments(FILE* fp){

	int curr_char=0;

	skip_comment:
	while(isspace((curr_char=fgetc(fp)))&&curr_char!=EOF);
	ungetc(curr_char,fp);

	if(curr_char==';'){
	
		if((curr_char=fgetc(fp))!=EOF){
			ungetc(curr_char,fp);
			while(((curr_char=fgetc(fp))!='\n')&&curr_char!=EOF);
			ungetc(curr_char,fp);
		}

	}

	while(isspace((curr_char=fgetc(fp)))&&curr_char!=EOF);
	ungetc(curr_char,fp);
	if(curr_char==';'){
		goto skip_comment;
	}
}
