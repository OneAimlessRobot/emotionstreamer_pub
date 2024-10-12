#ifndef CLIENT_H
#define CLIENT_H

#define NO_SUCH_FILE_IN_SERVER "Ficheiro não existe no server: 0 Bytes recebidos.\n"
#define FILE_CREATION_ERROR "Não foi possivel criar destino dos dados recebidos.\n"
#define CONTENT_PEEK_INCOMMING "CONTEUDOS DO SERVER SERAO IMPRIMIDOS! AGUARDE!\n"
#define MAX_TRIES_HIT_ERROR "Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n"
#define CONNECTION_ATTEMPT_NO_MSG "Tentando conectar a %s (Tentativa %d)!!!!!!\n"
#define CONNECTION_ESTABLISHED_MSG "Conectado a %s!!!!!!\n"
#define DOWNLOAD_SIZE_MSG "Tamanho de download; %ld bytes\n"
#define CREATED_FILE_PATH_MSG "FICHEIRO CRIADO TEM PATH: %s\n"
#define SOCK_ERROR_DUMP "Erro normal:%s\n Erro Socket%s\nNumero socket: %d\n"

/*
typedef struct cli_connection{


        char file_path[PATHSIZE];
        char from_addr_str[INET_ADDRSTRLEN];
        struct sockaddr_in con_from_addr, udp_from_addr;
        int fp,is_on,sockfd_tcp,sockfd_udp;
        u_int64_t data_size;
        socklen_t udp_addr_size;



}cli_connection;
*/
//Strings todas 0 ended
int clientStart(char* req_field,char* file_name,uint32_t udp_s_port, char* s_hostname);
#endif
