#ifndef PLAYER_H
#define PLAYER_H


#define FREQ 44000

#define CHUNK_BIG 4096
#define CHUNK_SMALL 1024
#define CHUNK_MID 2048

#define MUSIC_INFO_FORMAT "A musica tem as seguintes infos:\nNome de ficheiro: %s\nDuracao: %d s\nNome do autor: %s\nAlbum: %s\nTitulo: %s\n\nCopyright info: %s\n\n"
				
typedef struct thread_mtx_cond_struct{
	pthread_t thread_id;
	pthread_mutex_t mtx;
	int16_t stop_go;
	pthread_cond_t condvar;


}thread_mtx_cond_struct;

typedef struct thread_mtx_cond_struct * thread_mtx_cond_ref;

//String is null ended.
int player_init(const char* filename);



#endif
