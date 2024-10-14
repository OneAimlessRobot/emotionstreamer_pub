
#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/load_html.h"



static char* close_keyword = "end_of_contents.";
static char* tmpOne=".tmp.html",* tmpTwo=".tmp1.html";

static char* tmpDir=NULL,*tmpDir2=NULL,*currSearchedDir=NULL;

static void generateDirListingPrimitive(void){

	tmpDir=malloc(PATHSIZE);
        memset(tmpDir,0,PATHSIZE);
        tmpDir2=malloc(PATHSIZE);
        memset(tmpDir2,0,PATHSIZE);
        currSearchedDir=malloc(PATHSIZE);
        memset(currSearchedDir,0,PATHSIZE);
        snprintf(tmpDir,PATHSIZE,"%s%s",curr_dir,tmpOne);
        snprintf(tmpDir2,PATHSIZE,"%s%s",curr_dir,tmpTwo);
        int outfd= open(tmpDir,O_TRUNC|O_WRONLY|O_CREAT,0777);
        char* cmd= malloc(PATHSIZE);
        memset(cmd,0,PATHSIZE);
	snprintf(currSearchedDir,PATHSIZE,"%s",curr_dir);
        snprintf(cmd,PATHSIZE,"ls -1 %s > %s",currSearchedDir,tmpDir);
        system(cmd);
        memset(cmd,0,PATHSIZE);
	snprintf(cmd,PATHSIZE,"echo \"%s\" >> %s",close_keyword,tmpDir);
        system(cmd);
        free(cmd);
        close(outfd);
}



char* generateDirListing(void){
	logging=1;
	logstream=stderr;
	generateDirListingPrimitive();
	int fd=	open(tmpDir2,O_TRUNC|O_WRONLY|O_CREAT,0777);
	
	if(!tmpDir||!tmpDir2){
		if(logging){
			fprintf(logstream,"ERRO NAS DIRETORIAS!!!! Uma das listings esta nula!!!!\n");
		}
		return NULL;
	}
	if(fd<0){
		if(logging){
		fprintf(logstream,"ERRO NAS DIRETORIAS %s\n",tmpDir2);
		}
		return NULL;
	}
	FILE* fstream;
	if(!(fstream=fopen(tmpDir,"r"))){
		if(logging){
		fprintf(logstream,"ERRO NAS DIRETORIAS %s\n",tmpDir);
		}
		return NULL;
	}
	remove(tmpDir);
	char* currListing=malloc(BUFFSIZE);
	dprintf(fd,"Conteudos do server:\n");

	while(1){

		memset(currListing,0,BUFFSIZE);
		fgets(currListing,BUFFSIZE,fstream);

		currListing[strlen(currListing)-1]=0;
		if(!strs_are_strictly_equal(currListing,close_keyword)){
			break;
		}
		dprintf(fd,"%s\n",currListing);
	
	}
	free(currListing);
        close(fd);
        free(currSearchedDir);
        free(tmpDir);
        return tmpDir2;

}

void deleteDirListingFile(void){
char buff[PATHSIZE*2]={0};
snprintf(buff,PATHSIZE*2,"%s",tmpDir2);
remove(buff);

}
