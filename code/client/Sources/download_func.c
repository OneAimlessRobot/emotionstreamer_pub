#include "../../Includes/preprocessor.h"
#include <ncurses.h>
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../Includes/configs.h"
#include "../Includes/download_func.h"
static int fd,sock;

typedef struct download_bar{

        int curr,total,size;

}download_bar;



static download_bar bar={0};


static void* print_download_bar(void* mem,int len,int* timeout_num){

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
        erase();
        printw("Progresso atual de download: %d de %d kbytes transferidos!\n\n%s\n",bar_inside->curr/1000,bar_inside->total/1000,bar);
        if(len==-2){
		(*timeout_num)++;
		printw("Timeout no read!!!! Timeout no. %d\n",*timeout_num);
        }
	else{
		(*timeout_num)=0;
	}
	refresh();
        return mem;

}


int downloadtofd(int sock,int fd,int size,int_pair times){
        int32_t len=1;
        int32_t written=1;
	int32_t total=0;
	int32_t timeout_num=0;
        char buff[DEF_DATASIZE];
        bar.curr=0;
        bar.total=size;
        bar.size=40;
        memset(buff,0,DEF_DATASIZE);
        for(;(len==-2||len>0)&&(total!=size);){
		len=readsome(sock,buff,DEF_DATASIZE,times);
                if(len==-2){
			printw("Timeout no download!!!\n");
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
                fprintf(logstream,"readalltofd bem sucedido. A socket e %d\nLemos %d de %d bytes\n",sock,bar.curr,size);

        }
        memset(buff,0,DEF_DATASIZE);
        sendsome(sock,buff,DEF_DATASIZE,times);

        return 0;

}


