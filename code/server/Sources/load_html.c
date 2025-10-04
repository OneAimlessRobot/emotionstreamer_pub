#include "../../Includes/preprocessor.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/load_html.h"



static char* close_keyword = "end_of_contents.";
static char* tmpOne=".tmp.html",* tmpTwo=".tmp1.html";

static char tmpDir[PATHSIZE*2]={0},tmpDir2[PATHSIZE*2]={0},currSearchedDir[PATHSIZE*2]={0};

static void generateDirListingPrimitive(char* pattern){

        snprintf(tmpDir,PATHSIZE*2-1,"%s%s",curr_dir,tmpOne);
        snprintf(tmpDir2,PATHSIZE*2-1,"%s%s",curr_dir,tmpTwo);
        int outfd= open(tmpDir,O_TRUNC|O_WRONLY|O_CREAT,0777);
        char cmd[PATHSIZE*10]={0};
	snprintf(currSearchedDir,PATHSIZE*2-1,"%s",curr_dir);
	//THIS LINE HAS RIPPED CODE! FIND ALL BASEFILENAMES WITH EXTENSION '.WAV', but dont show the extension! (IMPORTANT FOR SECURITY)
	//https://www.baeldung.com/linux/find-filenames-no-extension
	//https://stackoverflow.com/questions/1447625/list-files-with-certain-extensions-with-ls-and-grep
        snprintf(cmd,PATHSIZE*10-1,"find %s/*%s* -name \"*%s%s*\" | xargs -I{} basename {} \"%s%s\" > %s",currSearchedDir,pattern,server_working_extension,is_wav_mode?"":BOUNDARY_FILE_EXT,server_working_extension,is_wav_mode?"":BOUNDARY_FILE_EXT,tmpDir);
        //END OF RIPPEDD CODE
	system(cmd);
        memset(cmd,0,PATHSIZE*10);
	snprintf(cmd,PATHSIZE*10-1,"echo \"%s\" >> %s",close_keyword,tmpDir);
        system(cmd);
        close(outfd);
}



char* generateDirListing(char* pattern){
	logging=1;
	logstream=stderr;
	generateDirListingPrimitive(pattern);
	int fd=	open(tmpDir2,O_TRUNC|O_WRONLY|O_CREAT,0777);
	
	if(!strnlen(tmpDir2,PATHSIZE*4-1)||!strnlen(tmpDir2,PATHSIZE*4-1)){
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
	char* currListing=malloc(PATHSIZE*4-1);
	dprintf(fd,"Conteudos do server:\n");

	while(1){

		memset(currListing,0,PATHSIZE*4-1);
		fgets(currListing,PATHSIZE*4-1,fstream);

		currListing[strlen(currListing)-1]=0;
		if(!strs_are_strictly_equal(currListing,close_keyword)){
			break;
		}
		dprintf(fd,"%s\n",currListing);
	
	}
	free(currListing);
        close(fd);
        return tmpDir2;

}

void deleteDirListingFile(void){
char buff[PATHSIZE*4]={0};
snprintf(buff,PATHSIZE*4-1,"%s",tmpDir2);
remove(buff);

}
