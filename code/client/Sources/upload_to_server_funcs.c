#include "../../Includes/preprocessor.h"
#include <ncurses.h>
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/sockio_tcp.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/configs.h"
#include <pulse/error.h>
#include <pulse/simple.h>
#include <sys/ioctl.h> //for ioctl()
#include <linux/soundcard.h> //SOUND_PCM*
#include <ao/ao.h>
#include <alsa/asoundlib.h>
#include "../Includes/configs.h"
#include "../Includes/terminal_mgmt.h"
#include "../Includes/ripped_code.h"
#include "../Includes/download_func.h"
#include "../Includes/upload_to_server_funcs.h"


typedef struct upload_bar{

        int64_t curr,total,size;

}upload_bar;



static upload_bar bar={0};


static void* print_upload_bar(void* mem){

        upload_bar* bar_inside= (void*)mem;

        char bar[bar_inside->size+3];
        memset(bar,0,bar_inside->size+3);
        bar[0]='[';
        bar[bar_inside->size+1]=']';
        for(int i=1;i<bar_inside->size+1;i++){

                bar[i]=' ';

        }
        for(int i=1;i<round((double)(bar_inside->curr*bar_inside->size)/(double)bar_inside->total);i++){

                bar[i]='#';

        }
	if(stream_enable_ncurses){
		move(0,0);
	}
	else{
		printf("\033[H");
	}
	char buff[1024]={0};
        snprintf(buff,sizeof(buff)-1,"Progresso atual de upload: %ld de %ld kbytes enviados!!\n\n%s\n",bar_inside->curr/1000,bar_inside->total/1000,bar);
        print_string(buff);
	if(stream_enable_ncurses){
		refresh();
	}
	return mem;

}

int upload_to_server_func(int sock,int fd,int total_size,int_pair times){

char buff[DEF_DATASIZE];
memset(buff,0,DEF_DATASIZE);
bar.curr=0;
bar.total=total_size;
bar.size=40;
int numread;
int all_that_was_sent=0;
int sent=0;
while ((numread = read(fd,buff,DEF_DATASIZE)) > 0) {

        errno=0;
        sent = sendsome(sock, buff,  numread,times);
        bar.curr+=(sent<0)? 0:sent;
	print_upload_bar((void*)&bar);
	memset(buff,0,DEF_DATASIZE);
        if(sent==-2){

                if(logging){
                fprintf(logstream,"Timeout no sending!!!!: %s\nsocket %d\n",strerror(errno),sock);
                }
                lseek(fd,-(numread-sent),SEEK_CUR);
                continue;
        }
        if(sent<0){

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if(logging){
                fprintf(logstream,"Block no sending!!!!: %s\nsocket %d\n",strerror(errno),sock);
                }
                break;

        }
        else if(errno==EPIPE){

                if(logging){
                fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
                }
                raise(SIGINT);
                return -1;
        }
        else if(errno == ECONNRESET){
                if(logging){
                fprintf(logstream,"Conexão largada!!\nSIGPIPE!!!!!: %s\n",strerror(errno));
                }
                raise(SIGINT);
                return -1;
        }
        else {
                if(logging){
                fprintf(logstream,"Outro erro qualquer!!!!!: %d %s\n",errno,strerror(errno));
                }
                break;
        }
        }
        	all_that_was_sent+=sent;
	}

        if(logging){
                fprintf(logstream,"send de %d bytes feito!!!!!\n",all_that_was_sent);
        }
        return 0;
}













