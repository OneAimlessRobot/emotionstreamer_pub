#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/auxfuncs.h"
#include "Includes/converter.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "Includes/configs.h"

int main(int argc, char ** argv){

	if(argc!=3){

		printf("Usage:\n"
				"Arg1: input filename (rel. to input_folder defined in config/sizes.cfg)\n"
				"Arg2: output filename (rel. to output_folder defined in config/sizes.cfg)\n");
		exit(-1);
	}

	read_values_cfg_converter();
        print_values_cfg_converter(1);
	memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);
	start_frame_info_machine(argv[1],argv[2],0);



	return 0;
}
