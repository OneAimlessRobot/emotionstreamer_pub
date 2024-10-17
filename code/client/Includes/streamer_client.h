#ifndef STREAMER_CLIENT_H
#define STREAMER_CLIENT_H

#define MUSIC_INFO_FORMAT "A musica tem as seguintes infos:\nNome de ficheiro: %s\nDuracao: %d s\nNome do autor: %s\nAlbum: %s\nTitulo: %s\n\nCopyright info: %s\n\n"

typedef struct stream_client_mem{

         int stream_on;
         int sockfd;
	 int curr_chk_index;
         Mix_Chunk*  chk;
         unsigned char chunk_data_cache[CHUNK_SIZE*STREAM_CACHE_SIZE_CHUNKS];
	 char server_reply_buff[DEF_DATASIZE+1];
         int_pair pair;


}stream_client_mem;


int player_init_stream(int server_sock);

#endif
