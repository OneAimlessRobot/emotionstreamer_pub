#ifndef STREAMER_CLIENT_H
#define STREAMER_CLIENT_H

#define MUSIC_INFO_FORMAT "A musica tem as seguintes infos:\nNome de ficheiro: %s\nDuracao: %d s\nNome do autor: %s\nAlbum: %s\nTitulo: %s\n\nCopyright info: %s\n\n"

typedef struct client_stream_t{


          int innited;
          struct con_t* con_obj;
          u_int64_t curr_chk_index;
          Mix_Chunk*  chk;
          unsigned char* chunk_data_cache;

}client_stream_t;

//BUFFS COME FROM THE OUTSIDE!
//DOES NOT FREE BUFFERS OR INITIALIZE BUFFS

//con_obj is innited

void player_init_stream(con_t* con_obj, unsigned char* buff);
void player_stop_stream(void);

#endif
