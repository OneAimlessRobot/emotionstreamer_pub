
#ifndef CLIENT_H
#define CLIENT_H

#define NO_SUCH_FILE_IN_SERVER "Ficheiro não existe no server: 0 Bytes recebidos.\n"
#define FILE_CREATION_ERROR "Não foi possivel criar destino dos dados recebidos.\n"
#define CONTENT_PEEK_INCOMMING "CONTEUDOS DO SERVER SERAO IMPRIMIDOS! AGUARDE!\n"
#define CREATED_FILE_PATH_MSG "FICHEIRO CRIADO TEM PATH: %s\n"
#define CONTENT_DOWNLOAD_INCOMMING "%d bytes DO SERVER SERAO TRANFERIDOS!! AGUARDE!\n"
#define STREAM_INCOMMING "%d bytes DO SERVER SERAO SERAM STREAMADOS!! AGUARDE!\n"
#define UNSUCESSFUL_DOWNLOAD "NAO FOI POSSIVEL TRANSFERIR FICHEIRO!!!!! %s\n"
#define UNSUCESSFUL_DOWNLOAD_NOFILE "FICHEIRO NAO EXISTE!\n"
#define UNSUCESSFUL_DOWNLOAD_CON_ERROR "ERRO CONEXAO\n"



//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,char* s_hostaddr);
#endif
