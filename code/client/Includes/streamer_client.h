#ifndef STREAMER_CLIENT_H
#define STREAMER_CLIENT_H

typedef struct client_stream_t{


          int innited;
	  uint64_t curr_timeout;
          struct con_t* con_obj;
	  chunk_queue* decoder_que;
	  chunk_queue* player_que;
	  chunk_queue* auxiliar_que;
	  chunk_player* player;
	  decoder* decoder;
}client_stream_t;

//BUFFS COME FROM THE OUTSIDE!
//DOES NOT FREE BUFFERS OR INITIALIZE BUFFS

//con_obj is innited

void player_init_stream(con_t* con_obj, uint64_t chunk_size, method which_mode);
void player_stop_stream(void);

#endif
