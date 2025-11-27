#ifndef PROTOCOL_H
#define PROTOCOL_H


typedef enum req_type{PLAY,CONF,PEEK,DOWN,REPORT,NA}req_type;

//todas as strings sao terminadas a 0
req_type str_to_req_type(char* str);


//conteudos sao completamente reescritos
void req_type_to_str(req_type type,char buff[]);



#define UNKNOWN_REQ "Request desconhecido: \"%s\"\nRequests válidos são:\npeek- Ver musicas no server.\nplay- tocar um ficheiro do server.\ndown- transferir um ficheiro do server\nreport- denunciar um ficheiro defeituoso do server\n"





#endif
