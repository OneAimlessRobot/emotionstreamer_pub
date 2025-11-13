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

        int outfd= open(tmpDir,O_TRUNC|O_WRONLY|O_CREAT,0777);
        char cmd[PATHSIZE*10]={0};
	snprintf(currSearchedDir,PATHSIZE*2-1,"%s",curr_dir);
	//THIS LINE HAS RIPPED CODE! FIND ALL BASEFILENAMES WITH EXTENSION '.WAV', but dont show the extension! (IMPORTANT FOR SECURITY)
	//https://www.baeldung.com/linux/find-filenames-no-extension
	//https://stackoverflow.com/questions/1447625/list-files-with-certain-extensions-with-ls-and-grep
	//https://stackoverflow.com/questions/41965415/using-find-to-return-filenames-without-extension
        //-iname "*.ipynb" -exec dirname "{}" ';' -exec basename "{}" .ipynb ';'
        ///* -exec dirname \"{}\" ';' -exec basename \"{}\" %s%s ';' > %s"*/
	//f=$(basename "$1" .ipynb);d=$(dirname "$1"); echo "$d/$f"
	//# Source - https://stackoverflow.com/a
	//find .  -type f -iname "*.ipynb" -exec sh -c 'f=$(basename $1 .ipynb);d=$(dirname $1);echo "$d/$f"' sh {} \;
	//"&& find . -iname '*%s*%s%s*' -exec sh -c 'printf \"%%s\\n\" \"${0%%.*}\"' {} ';' > %s "
	//One of these has to work, damn it

	snprintf(cmd,PATHSIZE*10-1,"bash -c \"pushd %s "
					"&& find .  -type f -iname '*%s*%s%s*'  > %s "
					"&& echo \"%s\" >> %s "
					"&& popd "
					"&& exit\" ",
					currSearchedDir,
					strlen(pattern)?pattern:"",
					server_working_extension,
					is_wav_mode?"":BOUNDARY_FILE_EXT,
					tmpDir,
					close_keyword,
					tmpDir);
	printf("%s\n",cmd);
	//END OF RIPPEDD CODE
	system(cmd);
        /*
	memset(cmd,0,PATHSIZE*10);
	snprintf(cmd,PATHSIZE*10-1,"echo \"%s\" >> %s",close_keyword,tmpDir);
        system(cmd);
	*/
        close(outfd);
}
char *get_file_extension(const char *path) {
	char *extension = strrchr(path, '.');
	char* extension2=NULL;
	if (extension != NULL) {
		if(is_wav_mode){

			return extension+1;
		}
		else{
			*(extension)=0;
		    	extension2 = strrchr(path, '.');
			if (extension2 != NULL) {
				*(extension)='.';
				*(extension2)=0;
				return extension2 + 1;
			}
		}
	}
	// No file extension found
	return NULL;
}



char* generateDirListing(char* pattern){
	logging=1;
	logstream=stderr;
        snprintf(tmpDir,PATHSIZE*2-1,"%s%s",curr_dir,tmpOne);
        snprintf(tmpDir2,PATHSIZE*2-1,"%s%s",curr_dir,tmpTwo);
	int fd=	open(tmpDir2,O_TRUNC|O_WRONLY|O_CREAT,0777);
	if(fd<0){
		if(logging){
		fprintf(logstream,"ERRO NAS DIRETORIAS %s\n",tmpDir2);
		}
		return NULL;
	}
	generateDirListingPrimitive(pattern);
	FILE* fstream;
	if(!(fstream=fopen(tmpDir,"r"))){
		if(logging){
		fprintf(logstream,"ERRO NAS DIRETORIAS %s\n",tmpDir);
		}
		close(fd);
		return NULL;
	}
	remove(tmpDir);
	char currListing[PATHSIZE*4-1]={0};
	dprintf(fd,"Conteudos do server:\n");

	while(1){

		memset(currListing,0,PATHSIZE*4-1);
		fgets(currListing,PATHSIZE*4-1,fstream);
		currListing[strlen(currListing)-1]=0;
		if(!strs_are_strictly_equal(currListing,close_keyword)){
			dprintf(fd,"%s\n\n\nServer contents successfully retrieved in full.\n\n",currListing);
			break;
		}
		char* ext_ptr=get_file_extension(currListing);
		if(ext_ptr){
			dprintf(fd,"%s\n",currListing);
		}
	}
        close(fd);
        return tmpDir2;

}

void deleteDirListingFile(void){
char buff[PATHSIZE*4]={0};
snprintf(buff,PATHSIZE*4-1,"%s",tmpDir2);
remove(buff);

}
