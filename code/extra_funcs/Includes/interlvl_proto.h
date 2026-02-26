#ifndef INTERLVL_PROTO_H
#define INTERLVL_PROTO_H

#define SHOW_STRING "showme"
#define SHOW_MASTER_STRING "showme_master"
#define LOG_STRING "mayi"



typedef enum interlvl_cmd{SHOW=14,MASTER_SHOW=15,LOG=20,INTERLVL_NA=25}interlvl_cmd;


interlvl_cmd str_to_interlvl_cmd_type(char* str);
void interlvl_cmd_type_to_str(interlvl_cmd type,char* buff);


#endif
