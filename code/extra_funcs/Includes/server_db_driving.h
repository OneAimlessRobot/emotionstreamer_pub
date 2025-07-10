#ifndef SERVER_DB_DRIVING_H
#define SERVER_DB_DRIVING_H

#define INSERT_SERVER_TMPL "INSERT Into Servers Values(%s)"

#define DELETE_SERVER_TMPL "DELETE from Servers\nWhere server_id = %d"

#define SHOW_SERVERS_TMPL "SELECT * from Servers;"

#define SPAWN_SERVER_TABLE "CREATE TABLE Servers(server_type Varchar(256), server_id int unique not null,server_name Varchar(256) unique not null, server_ip Varchar(256), primary key(server_id))"

#define REMOVE_SERVER_TABLE "DROP TABLE Servers"

#define SERVER_LIST_HEADER "------------------CABEÇALHO DA LISTA DE DOWNS DESTE NODO!!!!!!-----------------------"

//NULL TERMINATED
int openDB(char* dbname);

int insert_server(char* server_str);

int delete_server(int server_id);
int show_servers(con_t* con_obj,int_pair pair);

int closeDB(void);





#endif
