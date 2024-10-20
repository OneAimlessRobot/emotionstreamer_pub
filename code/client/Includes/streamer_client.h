#ifndef STREAMER_CLIENT_H
#define STREAMER_CLIENT_H

#define MUSIC_INFO_FORMAT "A musica tem as seguintes infos:\nNome de ficheiro: %s\nDuracao: %d s\nNome do autor: %s\nAlbum: %s\nTitulo: %s\n\nCopyright info: %s\n\n"

typedef struct client_stream_t{


          int innited;
          struct con_t* con_obj;
          int curr_chk_index;
          Mix_Chunk*  chk;
          unsigned char chunk_data_cache[STREAM_CACHE_SIZE_CHUNKS*CHUNK_SIZE];

}client_stream_t;


void player_init_stream(con_t* con_obj);
void player_stop_stream(void);

#endif
