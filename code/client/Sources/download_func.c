#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include <ncurses.h>
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

typedef struct download_bar{

        int64_t curr,total,size;

}download_bar;



static download_bar bar={0};


static void* print_download_bar(void* mem,int64_t len,int64_t* timeout_num){

        download_bar* bar_inside= (void*)mem;

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
        snprintf(buff,sizeof(buff)-1,"Progresso atual de download: %ld de %ld kbytes transferidos!\n\n%s\n",bar_inside->curr/1000,bar_inside->total/1000,bar);
        print_string(buff);
	if(len==-2){
		(*timeout_num)++;
		char buff[1024]={0};
        	snprintf(buff,sizeof(buff)-1,"Timeout no read!!!! Timeout no. %ld\n",*timeout_num);
        	print_string(buff);
	}
	else{
		(*timeout_num)=0;
	}
        if(stream_enable_ncurses){
		refresh();
	}
	return mem;

}


int downloadtofd(int sock,int fd,int64_t size,int_pair times,uint8_t is_ssl,SSL* cSSL){
        int64_t len=1;
        int64_t written=1;
	int64_t total=0;
	int64_t timeout_num=0;
        char buff[DEF_DATASIZE];
        bar.curr=0;
        bar.total=size;
        bar.size=40;
        memset(buff,0,DEF_DATASIZE);
        for(;(len==-2||len>0)&&(total!=size);){
		len=readsome(sock,buff,DEF_DATASIZE,times,is_ssl,cSSL);
                if(len==-2){
			printf("Timeout no download!!!\n");
                }
                written=write(fd,buff,len);
		total+= (written<0)? 0:written;
                bar.curr+=(written<0)? 0:written;
		print_download_bar((void*)&bar,len,&timeout_num);
                memset(buff,0,DEF_DATASIZE);
        }
        if(!(bar.curr)){
                if(logging){
                fprintf(logstream,"readall bem sucedido!! A socket e %d\n",sock);

                }
        }
        if(len<0){
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if(logging){
                fprintf(logstream,"readall bem sucedido!! A socket e %d\n",sock);
                }
        }
        else if(errno==EPIPE){

                if(logging){
                fprintf(logstream,"Pipe partido!!! A socket e %d\n",sock);
                }
                return -2;
        }
        else if(errno==ENOTCONN){
                if(logging){
                fprintf(logstream,"readall saiu com erro!!!!!:\nAvisando server para desconectar!\n%s\n",strerror(errno));
                }

                return -2;
        }
        else if(len!=-2){
                if(logging){
                fprintf(logstream,"readall saiu com erro!!!!!:\n%s\n",strerror(errno));
                }
        }

        }
        if(logging){
                fprintf(logstream,"readalltofd bem sucedido. A socket e %d\nLemos %ld de %ld bytes\n",sock,bar.curr,size);

        }
        memset(buff,0,DEF_DATASIZE);
        sendsome(sock,buff,DEF_DATASIZE,times,is_ssl,cSSL);

        return 0;

}


