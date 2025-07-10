#ifndef PROTOCOL_H
#define PROTOCOL_H


typedef enum req_type{PLAY,CONF,PEEK,DOWN,NA}req_type;
typedef enum stream_cmd{TOGGLE,STOP,REWIND,TIME,NOP}stream_cmd;

//todas as strings sao terminadas a 0
req_type str_to_req_type(char* str);


//conteudos sao completamente reescritos
void req_type_to_str(req_type type,char buff[]);

//todas as strings sao terminadas a 0
stream_cmd str_to_stream_cmd(char* str);


//conteudos sao completamente reescritos
void stream_cmd_to_str(stream_cmd type,char buff[]);


#define UNKNOWN_REQ "Request desconhecido: Requests válidos são:\npeek- Ver musicas no server.\nplay- tocar um ficheiro do server.\ndown- transferir um ficheiro do server\n"





#endif
