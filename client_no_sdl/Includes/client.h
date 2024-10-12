#ifndef CLIENT_H
#define CLIENT_H

#define UNKNOWN_REQ "Request desconhecido: Requests válidos são:\npeek- Ver ficheiros de audio dentro do server.\nget- Transferir e tocar um ficheiro do server.\n"
#define NO_SUCH_FILE_IN_SERVER "Ficheiro não existe no server: 0 Bytes recebidos.\n"
#define FILE_CREATION_ERROR "Não foi possivel criar destino dos dados recebidos.\n"
#define CONTENT_PEEK_INCOMMING "CONTEUDOS DO SERVER SERAO IMPRIMIDOS! AGUARDE!\n"
#define MAX_TRIES_HIT_ERROR "Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n"
#define CONNECTION_ATTEMPT_NO_MSG "Tentando conectar a %s (Tentativa %d)!!!!!!\n"
#define CONNECTION_ESTABLISHED_MSG "Conectado a %s!!!!!!\n"
#define DOWNLOAD_SIZE_MSG "Tamanho de download; %ld bytes\n"
#define CREATED_FILE_PATH_MSG "FICHEIRO CRIADO TEM PATH: %s\n"
#define SOCK_ERROR_DUMP "Erro normal:%s\n Erro Socket%s\nNumero socket: %d\n"

#define CLIENT_TIMEOUT_CON_SEC 1
#define CLIENT_TIMEOUT_CON_USEC 0

#define CLIENT_TIMEOUT_DATA_SEC 1
#define CLIENT_TIMEOUT_DATA_USEC 0
#define CLIENT_DATA_TIMES_PAIR ((int_pair){CLIENT_TIMEOUT_DATA_SEC,CLIENT_TIMEOUT_DATA_USEC})


//Strings todas 0 ended
int clientStart(char* req_field, char* file_name,u_int64_t port, char* host);

#endif
