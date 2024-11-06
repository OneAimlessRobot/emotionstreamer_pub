#ifndef STREAMER_CLIENT_H
#define STREAMER_CLIENT_H

typedef struct client_stream_t{


          int innited;
	  uint16_t curr_timeout;
          struct con_t* con_obj;
	  chunk_queue* que;
	  chunk_player* player;
}client_stream_t;

//BUFFS COME FROM THE OUTSIDE!
//DOES NOT FREE BUFFERS OR INITIALIZE BUFFS

//con_obj is innited

void player_init_stream(con_t* con_obj, uint16_t chunk_size, method which_mode);
void player_stop_stream(void);

#endif
