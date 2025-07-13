#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/protocol.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"


req_type str_to_req_type(char* str){

	
	if(!strs_are_strictly_equal(str,"play")){

		return PLAY;

	}
	if(!strs_are_strictly_equal(str,"peek")){

		return PEEK;

	}
	if(!strs_are_strictly_equal(str,"conf")){

		return CONF;

	}
	if(!strs_are_strictly_equal(str,"down")){

		return DOWN;

	}

	return NA;

}
void req_type_to_str(req_type type,char buff[]){

	memset(buff,0,DEF_DATASIZE);
	switch(type){

		case PLAY:
			snprintf(buff, DEF_DATASIZE, "play");
		break;
		case PEEK:
			snprintf(buff, DEF_DATASIZE, "peek");
		break;
		case DOWN:
			snprintf(buff, DEF_DATASIZE, "down");
		break;
		case CONF:
			snprintf(buff, DEF_DATASIZE, "conf");
		break;
		default:
			snprintf(buff, DEF_DATASIZE, "NONE");
		break;
	


	}
}

stream_cmd str_to_stream_cmd(char* str){

	
	if(!strs_are_strictly_equal(str,"toggle")){

		return TOGGLE;

	}
	if(!strs_are_strictly_equal(str,"time")){

		return TIME;

	}
	if(!strs_are_strictly_equal(str,"stop")){

		return STOP;
	}
	
	if(!strs_are_strictly_equal(str,"rewind")){

		return REWIND;
	}
	return NOP;

}
void stream_cmd_to_str(stream_cmd type,char buff[]){

	memset(buff,0,DEF_DATASIZE);
	switch(type){

		case TOGGLE:
			snprintf(buff, DEF_DATASIZE, "toggle");
		break;
		case STOP:
			snprintf(buff, DEF_DATASIZE, "stop");
		break;
		case REWIND:
			snprintf(buff, DEF_DATASIZE, "rewind");
		break;
		case TIME:
			snprintf(buff, DEF_DATASIZE, "time");
		break;
		default:
			snprintf(buff, DEF_DATASIZE, "NONE");
		break;
	


	}
}
