#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../converter_tool/Includes/converter.h"
#include "../../extra_funcs/Includes/streamer_const.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../Includes/load_html.h"
#include "../Includes/configs.h"

static FILE* cfg_fp=NULL;
static FILE* rotation_file_stream=NULL;
static int tmp_cfg_fd=-1;
static int tmp_rotation_fd=-1;
static char curr_line_buff[CONFIG_READ_LINE_BUFF_SIZE]={0};

static char server_ip_address_buff[PATHSIZE+1]={0};
static char upper_ip_address_buff[PATHSIZE+1]={0};
static char server_port_mapper_ip_address_buff[PATHSIZE+1]={0};
char generalized_config_filepath_buff[PATHSIZE+1]={0};
char server_name_buff[PATHSIZE+1]={0};
ip_cache_entry server_ip_cache_entry={{0},0};
ip_cache_entry upper_ip_cache_entry={{0},0};
ip_cache_entry server_port_mapper_ip_cache_entry={{0},0};

char server_music_folder_path[PATHSIZE+1]={0};
char server_music_quarantine_folder_path[PATHSIZE+1]={0};
char curr_server_quarantine_dir_buff[PATHSIZE+1]={0};

char server_working_extension[EXTENSION_SIZE]={0};
const uint8_t server_display_splash=1;

uint8_t cfg_server_logging=0;

uint8_t cfg_server_slave_mode=1;
char server_auto_mode_rotation[ROTATION_LENGTH_LIMIT][ROTATION_SONG_FILENAME_LENGTH]={{0}};
char server_auto_mode_rotation_filename[CONFIG_READ_LINE_BUFF_SIZE]={0};
unsigned int is_auto_mode=0,
           curr_num_songs_rotation=0,
           curr_song_index_rotation=0;

struct timeval rotation_period={DEFAULT_ROTATION_PERIOD,0},
		curr_song_waited_time={0,0};
//EM BYTES E HZ!

int_pair server_data_times_pair=(int_pair){SERVER_TIMEOUT_DATA_SEC,SERVER_TIMEOUT_DATA_USEC};
int_pair server_con_times_pair=(int_pair){SERVER_TIMEOUT_CON_SEC,SERVER_TIMEOUT_CON_USEC};
int_pair server_ack_times_pair=(int_pair){SERVER_TIMEOUT_ACK_SEC,SERVER_TIMEOUT_ACK_USEC};
int_pair server_drop_chunks_times_pair=(int_pair){SERVER_DROP_CHUNK_TIMEOUT_SEC,SERVER_DROP_CHUNK_TIMEOUT_USEC};

uint64_t cfg_server_ack_period_us=DEF_SERVER_ACK_PERIOD_US;


uint64_t server_chunk_size=SERVER_CHUNK_SIZE;
int8_t is_wav_mode=1;
static void clean_buff(void){

	memset(&curr_line_buff,0,CONFIG_READ_LINE_BUFF_SIZE);

}

static void process_ip_cache_entries(void){

        parse_ip_cache_entry(server_ip_address_buff,&server_ip_cache_entry);
        parse_ip_cache_entry(upper_ip_address_buff,&upper_ip_cache_entry);
        parse_ip_cache_entry(server_port_mapper_ip_address_buff,&server_port_mapper_ip_cache_entry);

}


static void clean_and_exit(void){
	if(cfg_fp) {
		fclose(cfg_fp);
	}
	if(rotation_file_stream) {
		fclose(rotation_file_stream);
	}
	printf("Saimos no leitor de cfg. do server. Erro: %s\nPath para config: %s\n",strerror(errno),CONFIG_FILE_PATH_SERVER);
	exit(-1);
}
static void prepare_nightmare_blunt_rotation(void){

	if(is_auto_mode){
		printf("The server was launched in auto mode!\nPreparing rotation...\n");
	        if(!(rotation_file_stream=fopen(server_auto_mode_rotation_filename,"r"))){
			if(!(rotation_file_stream=fopen(ROTATION_FILE_PATH_SERVER,"r"))){
				fprintf(stderr,"Could not even open rotation file"
						"from compile time constant!!!\n"
						"Config value was not valid!\n"
						"Error: %s\n",
						strerror(errno));
				clean_and_exit();
			}
			fprintf(stderr,"Opened rotation file from compile time constant!!!\n"
					"Config value was not valid!\n"
					"Error: %s\n",
					strerror(errno));
		}
		if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,rotation_file_stream))){
                	clean_and_exit();
        	}
        	sscanf(curr_line_buff,"%lu",&(rotation_period.tv_sec));
		printf("The rotation period will be %lu seconds!\n",(rotation_period.tv_sec));
	        if(((uint32_t)(rotation_period.tv_sec))<=(server_con_times_pair[0]+1)){
			fprintf(stderr,"The rotation time is equal or less\nThan the ammount of connection timeout seconds +1!\n(which is: %lu +1)!\nIllegal value: Exiting...\n",server_con_times_pair[0]);
			clean_and_exit();

		}
		clean_buff();
		char curr_auto_dir[PATHSIZE]={0};
		memset(curr_auto_dir,0,PATHSIZE);
		getcwd(curr_auto_dir,PATHSIZE);
		int result= strnlen(server_music_folder_path,PATHSIZE);
		if(!result){

			snprintf(curr_auto_dir+strlen(curr_auto_dir),PATHSIZE+1,"%s",MUSIC_SERVER_INPUT_PATH);

		}
		else{

			snprintf(curr_auto_dir+strlen(curr_auto_dir),PATHSIZE+1,"%s",server_music_folder_path);

		}
		char curr_rotation_buff[ROTATION_SONG_FILENAME_LENGTH]={0};
		curr_num_songs_rotation=0;
		curr_song_index_rotation=0;
		for(int i=0;i<ROTATION_LENGTH_LIMIT;i++){
			memset(curr_rotation_buff,0,sizeof(curr_rotation_buff));
			fgets(curr_rotation_buff,sizeof(curr_rotation_buff)-2,rotation_file_stream);
			curr_rotation_buff[strlen(curr_rotation_buff)-1]=0;
			if(!strnlen(curr_rotation_buff,ROTATION_SONG_FILENAME_LENGTH)){
				printf("There are no more songs in the rotation!\n");
				break;
			}
			FILE* tmp_auto=NULL;
			char test_filepath_auto[PATHSIZE*2+3]={0};
			snprintf(test_filepath_auto,sizeof(test_filepath_auto)-1,"%s%s",curr_auto_dir,curr_rotation_buff);

			if(!(tmp_auto=fopen(test_filepath_auto,"r"))){
				fprintf(stderr,"Could not open the file:"
						"\n%s\n"
						"from rotation!\n"
						"Error: %s\n",
						test_filepath_auto,
						strerror(errno));
				continue;
			}
			else{
				printf("Maybe adding song %s!\nThe current number of songs is: %d\n",
					test_filepath_auto,
					curr_num_songs_rotation);
				fclose(tmp_auto);
			}
			char* ext_ptr=get_file_extension(curr_rotation_buff);
			if(!strs_are_strictly_equal(ext_ptr,server_working_extension+1)){
				memcpy(server_auto_mode_rotation[curr_num_songs_rotation],curr_rotation_buff,sizeof(curr_rotation_buff)-1);
				curr_num_songs_rotation++;
				printf("Adding song %s!\nThe current number of songs is: %d\n",
					&(server_auto_mode_rotation[curr_num_songs_rotation-1][0]),
					curr_num_songs_rotation);
			}
			else{
				printf("Skipping song %s!\n"
					"The file extension does not match the servers'\n"
					"The server's working extension is: %s\n"
					"But the extracted extension was: %s\n",
					curr_rotation_buff,
					server_working_extension,
					ext_ptr);

			}
		}
		if(!curr_num_songs_rotation){
			printf("Attempted to run auto_mode without any valid songs! Exiting...\n");
			clean_and_exit();
        	}
	fclose(rotation_file_stream);

	}

}
void read_values_cfg_server(void){


	if(!(cfg_fp=fopen(CONFIG_FILE_PATH_SERVER,"r"))){

		clean_and_exit();
	}
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
	}
	sscanf(curr_line_buff,"server_logging: %hhu",&cfg_server_logging);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
	}
	sscanf(curr_line_buff,"server_chunk_size: %lu",&server_chunk_size);
	server_chunk_size=max(0,min(MAX_MP3_STREAM_CHUNK_BUFF_SIZE,server_chunk_size));
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
 	}
	sscanf(curr_line_buff,"server_timeouts_con: %lu %lu",&server_con_times_pair[0],&server_con_times_pair[1]);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
 	}
	sscanf(curr_line_buff,"server_timeouts_data: %lu %lu",&server_data_times_pair[0],&server_data_times_pair[1]);
	clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
 	}
	sscanf(curr_line_buff,"server_timeouts_ack: %lu %lu",&server_ack_times_pair[0],&server_ack_times_pair[1]);
	clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_ack_period_us: %lu",&cfg_server_ack_period_us);
        clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
 	}
	sscanf(curr_line_buff,"server_timeouts_drop_chunks: %lu %lu",&server_drop_chunks_times_pair[0],&server_drop_chunks_times_pair[1]);
	clean_buff();

	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
	}
	sscanf(curr_line_buff,"server_music_folder_path: %s",server_music_folder_path);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

	        clean_and_exit();
	}
	sscanf(curr_line_buff,"server_music_quarantine_folder_path: %s",server_music_quarantine_folder_path);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

	        clean_and_exit();
	}
	sscanf(curr_line_buff,"server_working_extension: %s",server_working_extension);
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_ip_address: %s",server_ip_address_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"upper_server_ip_address: %s",upper_ip_address_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_port_mapper_ip_address: %s",server_port_mapper_ip_address_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"generalized_config_filepath: %s",generalized_config_filepath_buff);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_is_auto_mode: %u",&is_auto_mode);
        clean_buff();
        if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"rotation_filepath_if_auto: %s",server_auto_mode_rotation_filename);
	prepare_nightmare_blunt_rotation();
	clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_name: %s",server_name_buff);
        clean_buff();
	if(!(fgets(curr_line_buff,CONFIG_READ_LINE_BUFF_SIZE,cfg_fp))){

                clean_and_exit();
        }
        sscanf(curr_line_buff,"server_is_slave_mode: %hhu",&cfg_server_slave_mode);
        clean_buff();
	fclose(cfg_fp);
	server_working_extension[sizeof(server_working_extension)-1]=0;
	server_music_folder_path[sizeof(server_music_folder_path)-1]=0;
	process_ip_cache_entries();


}


void produce_config_file(void){
	tmp_cfg_fd=open(TMP_CONFIG_FILE_PATH,O_WRONLY|O_CREAT|O_TRUNC,0777);
	if(tmp_cfg_fd<0){
		perror("Nao foi possivel abrir ficheiro de configs do server");
		return;
	}
	print_values_cfg_server(tmp_cfg_fd);
	close(tmp_cfg_fd);

}


void produce_rotation_file(void){
	tmp_rotation_fd=open(TMP_ROTATION_FILE_PATH,O_WRONLY|O_CREAT|O_TRUNC,0777);
	if(tmp_rotation_fd<0){
		perror("Nao foi possivel abrir ficheiro de rotations do server");
		return;
	}
	dprintf(tmp_rotation_fd,"This server is currently: %s auto mode.\n"
					"Currently: \"%u\" songs in rotation\n"
					"Rotation time: \"%lu\" seconds\n"
					"The current song in the rotation is: %s\n"
					"Which is song number %u\n\n",
					is_auto_mode?"in":"not in",
					is_auto_mode?curr_num_songs_rotation:0,
					is_auto_mode?(rotation_period.tv_sec):0,
					is_auto_mode?server_auto_mode_rotation[curr_song_index_rotation]:"None.",
					is_auto_mode?curr_song_index_rotation:0);

	time_spec_print_function(tmp_rotation_fd, "\nCurrent waited time: ", &curr_song_waited_time);
	for(uint32_t i=0;i<curr_num_songs_rotation;i++){
		dprintf(tmp_rotation_fd,"Song %d: %s\n",i,server_auto_mode_rotation[i]);
	}
	close(tmp_rotation_fd);

}

void print_values_cfg_server(int fd){

	dprintf(fd,"server_logging: %hhu\n",cfg_server_logging);

	dprintf(fd,"server_chunk_size: %lu (max: %u)\n",server_chunk_size,MAX_MP3_STREAM_CHUNK_BUFF_SIZE);

	dprintf(fd,"server_timeouts_data: %lus %lu us\n",server_data_times_pair[0],server_data_times_pair[1]);

	dprintf(fd,"server_timeouts_con: %lus %lu us\n",server_con_times_pair[0],server_con_times_pair[1]);

	dprintf(fd,"server_timeouts_ack: %lus %lu us\n",server_ack_times_pair[0],server_ack_times_pair[1]);

	dprintf(fd,"server_ack_period_us: %luus\n",cfg_server_ack_period_us);

	dprintf(fd,"server_timeouts_drop_chunks: %lus %lu us\n",server_drop_chunks_times_pair[0],server_drop_chunks_times_pair[1]);

	dprintf(fd,"server_music_folder_path: %s\n",server_music_folder_path);

	dprintf(fd,"server_music_quarantine_folder_path: %s\n",server_music_quarantine_folder_path);

	dprintf(fd,"server_working_extension: %s\n",server_working_extension);

        dprintf(fd,"generalized_config_filepath: %s\n",generalized_config_filepath_buff);

	dprintf(fd,"server_is_auto_mode: %u\n",is_auto_mode);

        dprintf(fd,"rotation_filepath_if_auto: %s\n",server_auto_mode_rotation_filename);

        dprintf(fd,"server_name: %s\n",server_name_buff);

        dprintf(fd,"server_is_slave_mode: %hhu\n",cfg_server_slave_mode);

	print_ip_cache_entry_fd(fd,&server_ip_cache_entry);

	print_ip_cache_entry_fd(fd,&upper_ip_cache_entry);

	print_ip_cache_entry_fd(fd,&server_port_mapper_ip_cache_entry);



}
