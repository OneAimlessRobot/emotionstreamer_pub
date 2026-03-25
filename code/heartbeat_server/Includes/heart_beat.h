#ifndef HEART_BEAT_H
#define HEART_BEAT_H


#define MAX_SERVERS 1000
#define HB_SERVER_NAME_SIZE 150
#define DB_FILE "./serverdb/servers.db"




void start_heart_beats(ip_cache_entry* ent_this,ip_cache_entry* ent_upper);




#endif

