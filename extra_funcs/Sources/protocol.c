#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/protocol.h"
#include "../Includes/fileshit.h"


req_type str_to_req_type(char* str){

	
	if(!strs_are_strictly_equal(str,"play")){

		return PLAY;

	}
	if(!strs_are_strictly_equal(str,"peek")){

		return PEEK;

	}

	return NA;

}
void req_type_to_str(req_type type,char buff[BUFFSIZE]){

	memset(buff,0,BUFFSIZE);
	switch(type){

		case PLAY:
			snprintf(buff, BUFFSIZE, "play");
		break;
		case PEEK:
			snprintf(buff, BUFFSIZE, "peek");
		break;
		default:
			snprintf(buff, BUFFSIZE, "NONE");
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
void stream_cmd_to_str(stream_cmd type,char buff[BUFFSIZE]){

	memset(buff,0,BUFFSIZE);
	switch(type){

		case TOGGLE:
			snprintf(buff, BUFFSIZE, "toggle");
		break;
		case STOP:
			snprintf(buff, BUFFSIZE, "stop");
		break;
		case REWIND:
			snprintf(buff, BUFFSIZE, "rewind");
		break;
		case TIME:
			snprintf(buff, BUFFSIZE, "time");
		break;
		default:
			snprintf(buff, BUFFSIZE, "NONE");
		break;
	


	}
}
