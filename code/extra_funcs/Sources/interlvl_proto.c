#include "../../Includes/preprocessor.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../Includes/interlvl_proto.h"


interlvl_cmd str_to_interlvl_cmd_type(char* str){


        if(!strs_are_strictly_equal(str,SHOW_STRING)){

                return SHOW;

        }
	if(!strs_are_strictly_equal(str,SHOW_MASTER_STRING)){

                return MASTER_SHOW;

        }
        if(!strs_are_strictly_equal(str,LOG_STRING)){

                return LOG;

        }

        return INTERLVL_NA;

}
void interlvl_cmd_type_to_str(interlvl_cmd type,char*buff){

        memset(buff,0,DEF_DATASIZE);
        switch(type){

                case MASTER_SHOW:
                        snprintf(buff, DEF_DATASIZE, SHOW_MASTER_STRING);
                break;
                case LOG:
                        snprintf(buff, DEF_DATASIZE, LOG_STRING);
                break;
                case SHOW:
                        snprintf(buff, DEF_DATASIZE, SHOW_STRING);
                break;
                default:
                        snprintf(buff, DEF_DATASIZE, "NONE");
                break;



        }
}





