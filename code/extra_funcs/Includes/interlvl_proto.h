#ifndef INTERLVL_PROTO_H
#define INTERLVL_PROTO_H

#define SHOW_STRING "showme"
#define SHOW_MASTER_STRING "showme_master"
#define LOG_STRING "mayi"

typedef enum interlvl_cmd{SHOW,MASTER_SHOW,LOG,INTERLVL_NA}interlvl_cmd;


interlvl_cmd str_to_interlvl_cmd_type(char* str);
void interlvl_cmd_type_to_str(interlvl_cmd type,char* buff);


#endif
