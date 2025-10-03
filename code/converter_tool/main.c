#include "../Includes/preprocessor.h"
#include "Includes/converter.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "Includes/configs.h"

int main(int argc, char ** argv){


	read_values_cfg_converter();
        print_values_cfg_converter(1);
	memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);



	return 0;
}
