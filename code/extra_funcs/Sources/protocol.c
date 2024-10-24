#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/protocol.h"
#include "../Includes/fileshit.h"
#include "../Includes/sockio.h"
#include "../Includes/configs.h"


req_type str_to_req_type(char* str){

	
	if(!strs_are_strictly_equal(str,"play")){

		return PLAY;

	}
	if(!strs_are_strictly_equal(str,"peek")){

		return PEEK;

	}
	if(!strs_are_strictly_equal(str,"down")){

		return DOWN;

	}

	return NA;

}
void req_type_to_str(req_type type,char buff[]){

	memset(buff,0,cfg_datasize);
	switch(type){

		case PLAY:
			snprintf(buff, cfg_datasize, "play");
		break;
		case PEEK:
			snprintf(buff, cfg_datasize, "peek");
		break;
		case DOWN:
			snprintf(buff, cfg_datasize, "down");
		break;
		default:
			snprintf(buff, cfg_datasize, "NONE");
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

	memset(buff,0,cfg_datasize);
	switch(type){

		case TOGGLE:
			snprintf(buff, cfg_datasize, "toggle");
		break;
		case STOP:
			snprintf(buff, cfg_datasize, "stop");
		break;
		case REWIND:
			snprintf(buff, cfg_datasize, "rewind");
		break;
		case TIME:
			snprintf(buff, cfg_datasize, "time");
		break;
		default:
			snprintf(buff, cfg_datasize, "NONE");
		break;
	


	}
}
