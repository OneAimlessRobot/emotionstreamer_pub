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

		return CONFIG;

	}
	if(!strs_are_strictly_equal(str,"rotation")){

		return ROTATION;

	}
	if(!strs_are_strictly_equal(str,"down")){

		return DOWN;

	}
	if(!strs_are_strictly_equal(str,"report")){

		return REPORT;

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
		case REPORT:
			snprintf(buff, DEF_DATASIZE, "report");
		break;
		case CONFIG:
			snprintf(buff, DEF_DATASIZE, "conf");
		break;
		case ROTATION:
			snprintf(buff, DEF_DATASIZE, "rotation");
		break;
		default:
			snprintf(buff, DEF_DATASIZE, "NONE");
		break;


	}
}
