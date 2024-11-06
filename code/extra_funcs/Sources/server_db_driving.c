#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/connection.h"
#include <sqlite3.h>
#include "../Includes/server_db_driving.h"


typedef struct servers_sender{

        int_pair pair;
	con_t con_obj;
}servers_sender;




static sqlite3 *db=NULL;

static void error_mgmnt_sql_stmt(sqlite3_stmt* stmt, int error){


        if(error!=SQLITE_DONE){

                fprintf(stderr,"Pera pera pera...\n");
                error=sqlite3_step(stmt);
                fprintf(stderr,"Ahhhhh...\n");
        }
        switch(error){

                case SQLITE_DONE:
                        fprintf(stderr,"Done. %s\n",sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        break;
                case SQLITE_ERROR:
                        fprintf(stderr,"Erro em pedido á base de dados: %s\n",sqlite3_errmsg(db));
                        break;
                case SQLITE_INTERRUPT:
                        fprintf(stderr,"Erro em pedido á base de dados: %s\n",sqlite3_errmsg(db));
                        break;
                case SQLITE_SCHEMA:
                        fprintf(stderr,"Erro em pedido á base de dados: %s\n",sqlite3_errmsg(db));
                        break;
                case SQLITE_CORRUPT:
                        fprintf(stderr,"Erro em pedido á base de dados: %s\n",sqlite3_errmsg(db));
                        break;

        }



}
static void drop_servers_table(void){

        char statement_str[PATHSIZE]={0};
        sqlite3_stmt* stmt;
        snprintf(statement_str,PATHSIZE-1,REMOVE_SERVER_TABLE);
        printf("Statement: %s\n",statement_str);
        sqlite3_prepare_v2(db,statement_str,PATHSIZE-1,&stmt,NULL);
        int result=sqlite3_step(stmt);
        error_mgmnt_sql_stmt(stmt,result);
        sqlite3_finalize(stmt);
}

static void cre8_servers_table(void){

        char statement_str[PATHSIZE]={0};
        sqlite3_stmt* stmt;
        snprintf(statement_str,PATHSIZE-1,SPAWN_SERVER_TABLE);
        printf("Statement: %s\n",statement_str);
        sqlite3_prepare_v2(db,statement_str,PATHSIZE-1,&stmt,NULL);
        int result=sqlite3_step(stmt);
        error_mgmnt_sql_stmt(stmt,result);
        sqlite3_finalize(stmt);
}
int openDB(char* dbname){

    int rc;

   rc = sqlite3_open_v2(dbname, &db,SQLITE_OPEN_FULLMUTEX|SQLITE_OPEN_READWRITE,NULL);

   if( rc ) {
      fprintf(stderr, "Can't open database %s: %s\n",dbname, sqlite3_errmsg(db));
      return 0;
   } else {
      fprintf(stderr, "Opened database successfully\n");
   }

   drop_servers_table();
   cre8_servers_table();

   return 1;
}

int closeDB(void){

   if(db){
        drop_servers_table();
        sqlite3_close_v2(db);
        fprintf(stderr, "Closed database successfully\n");
        db=NULL;
   }
   else{

        fprintf(stderr, "Did not close database!!! (Database already NULL)\n");

   }
   return 1;
}

int insert_server(char* server_str){

        char statement_str[PATHSIZE]={0};
        sqlite3_stmt* stmt;



        snprintf(statement_str,PATHSIZE-1,INSERT_SERVER_TMPL,server_str);
        printf("Statement: %s\n",statement_str);

        sqlite3_prepare_v2(db,statement_str,PATHSIZE-1,&stmt,NULL);

        int result=sqlite3_step(stmt);
        error_mgmnt_sql_stmt(stmt,result);
        sqlite3_finalize(stmt);

        return 1;



}
int delete_server(int server_id){

        char statement_str[PATHSIZE]={0};
        sqlite3_stmt* stmt;

        snprintf(statement_str,PATHSIZE-1,DELETE_SERVER_TMPL,server_id);
        printf("Statement: %s\n",statement_str);

        sqlite3_prepare_v2(db,statement_str,PATHSIZE-1,&stmt,NULL);

        int result=sqlite3_step(stmt);
        error_mgmnt_sql_stmt(stmt,result);
        sqlite3_finalize(stmt);
        return 1;

}
static int send_show_servers(void* arg1, int n_cols, char** cols, char ** cols_names){

        servers_sender* sender= (servers_sender*)arg1;
        char buff[DEF_DATASIZE-1]={0};
        char* ptr=buff;
        int curr_col=0;

        while(curr_col!=n_cols){

                ptr+=snprintf(ptr,(int)((buff+sizeof(buff)-1)-ptr),"%s ",cols[curr_col]);
                curr_col++;
        }
        clear_con_data(&sender->con_obj);
        snprintf((char*)sender->con_obj.udp_data,DEF_DATASIZE-1,"%s",buff);
        int result=con_send_udp(&sender->con_obj,sender->pair);
        if(result<0){
		if(result!=-2){
 	               perror("Erro a enviar servers a browser 3 send!!!!!!\n");
        	       raise(SIGINT);
		}
		else{
	               perror("Timeout a enviar servers a browser 3 send!!!!!!\n");
        	
		}
        }
	
        result=con_read_udp(&sender->con_obj,sender->pair);
        if(result<0){
		if(result!=-2){
 	                perror("Erro a enviar servers a browser 3 read!!!!!!\n");
	                raise(SIGINT);
		}
		else{
	                perror("Timeout a enviar servers a browser 3 read!!!!!!\n");
		}

        }

        return 0;

}
int show_servers(con_t* con_obj,int_pair pair){

servers_sender sender;
memcpy(&sender.con_obj,con_obj,sizeof(con_t));
memcpy(sender.pair,pair,sizeof(int_pair));

char statement_str[PATHSIZE]={0};

snprintf((char*)sender.con_obj.udp_data,DEF_DATASIZE-1,SERVER_LIST_HEADER);
int result=con_send_udp(&sender.con_obj,pair);
if(result<0){
	if(result!=-2){
		perror("Erro a enviar servers a browser 1 send!!!!!!\n");
		raise(SIGINT);
	}
	else{
	     	perror("Timeout a servers a browser 1 send!!!!!!\n");
	}

}
result=con_read_udp(&sender.con_obj,pair);
if(result<0){
	if(result!=-2){
		perror("Erro a enviar servers a browser 2 read!!!!!!\n");
		raise(SIGINT);
	}
	else{
	     	perror("Timeout a servers a browser 2 read!!!!!\n");
	}
}

snprintf(statement_str,PATHSIZE-1,SHOW_SERVERS_TMPL);

sqlite3_exec(db, statement_str,send_show_servers,(void*)&sender,NULL);

snprintf((char*)sender.con_obj.udp_data,DEF_DATASIZE-1,"done");
con_send_udp(&sender.con_obj,pair);

return 1;
}

