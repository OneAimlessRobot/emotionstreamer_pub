#include "../../Includes/preprocessor.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/configs.h"
#include <openssl/ssl.h>
#include "../../extra_funcs/Includes/fileshit.h"
#include "../Includes/load_html.h"
#include <sys/wait.h>

#define tmpOne "%s/.emotionstreamer_dir_find_results.tmp%lx.html"
#define tmpTwo "%s/.emotionstreamer_dir_find_results.tmp1%lx.html"

static char* close_keyword = "end_of_contents;";


static char tmpDir[PATHSIZE*2]={0},tmpDir2[PATHSIZE*2]={0},currSearchedDir[PATHSIZE*2]={0};

static void generateDirListingPrimitive(char* pattern){

        int outfd= open(tmpDir,O_TRUNC|O_WRONLY|O_CREAT,0777);
        snprintf(currSearchedDir,PATHSIZE*2-1,"%s",curr_dir);

	//THIS LINE HAS RIPPED CODE! FIND ALL BASEFILENAMES WITH THE USED EXTENSION, but dont show the extension! (IMPORTANT FOR SECURITY)
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
	//I was not proud of how I got this final solutiuon.
	//AT ALL.
	//(Ahem chatgpt solution ahem)
	//(Ahem I wish system() did not even exist so I would not be tempted to use it ahem)


	//We can sort now yippieeee
	//I understand this stuff much better after actually grappling with it :P

	char buff[PATHSIZE+5]={0};


	snprintf(buff,sizeof(buff)-3,".%s",server_music_folder_path);
	chdir(buff);
	memset(buff,0,sizeof(buff));
	char* pattern_arg= (strlen(pattern)?pattern:"");
	char* extension_arg1=(server_working_extension);
	char* extension_arg2=(is_wav_mode?"":BOUNDARY_FILE_EXT);
	snprintf(buff,sizeof(buff)-3,"*%s*%s*%s*",pattern_arg,extension_arg1,extension_arg2);
	char* args_cmd_find[]={"find",".", "-type","f","-iwholename", buff,NULL},
		*args_cmd_sort[]={"sort",NULL};
	pid_t pid_fork_find_cmd,
		pid_fork_sort_cmd;
	int the_pipe[2],pipe_result;
	pid_fork_sort_cmd=fork();
	switch(pid_fork_sort_cmd){

		case -1:
			close(outfd);
			exit(-1);
		break;
		case 0:
			pipe_result=pipe(the_pipe);
			if(pipe_result>=0){

				pid_fork_find_cmd=fork();
				switch(pid_fork_find_cmd){
					case -1:
						exit(-1);
					break;
					case 0:
						close(outfd);
						dup2(the_pipe[1],STDOUT_FILENO);
						close(the_pipe[0]);
						execvp(args_cmd_find[0],args_cmd_find);
					break;
					default:
					break;

				}
			}
			dup2(the_pipe[0],STDIN_FILENO);
			close(the_pipe[1]);
			dup2(outfd,STDOUT_FILENO);
			close(outfd);
			execvp(args_cmd_sort[0],args_cmd_sort);
		break;
		default:
			wait(NULL);
			dprintf(outfd,"%s\n",close_keyword);
		break;

	}
	/*
	my god I hate how easy it is to do just this instead of execvp
	fuck.
	you.
	system().
	I was about to go to bed, too.
	Fuck you.
	Die.
	Piece of shit.
	Monstro.
	Negro.
	Urso.
	besta.
	Carraça
	Cão.
	Camelo.
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
                *(extension)=0;
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
        snprintf(tmpDir,PATHSIZE*2-1,tmpOne,server_tmp_dir_path,curr_request_id);
        snprintf(tmpDir2,PATHSIZE*2-1,tmpTwo,server_tmp_dir_path,curr_request_id);
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
	while(1){

		memset(currListing,0,PATHSIZE*4-1);
		fgets(currListing,PATHSIZE*4-1,fstream);
		currListing[strlen(currListing)-1]=0;
		if(!strs_are_strictly_equal(currListing,close_keyword)){
			break;
		}
		char* ext_ptr=get_file_extension(currListing);
		if(ext_ptr){
			dprintf(fd,"%s\n",currListing);
		}
	}
        fclose(fstream);
	close(fd);
        return tmpDir2;

}

void deleteDirListingFile(void){
char buff[PATHSIZE*4]={0};
snprintf(buff,PATHSIZE*4-1,"%s",tmpDir2);
remove(buff);

}
