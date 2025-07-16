#include "../Includes/preprocessor.h"
#include "../extra_funcs/Includes/fileshit.h"
#include "../extra_funcs/Includes/sockio.h"
#include "../extra_funcs/Includes/connection.h"
#include "../extra_funcs/Includes/ip_cache_file.h"
#include "../extra_funcs/Includes/generalized_config.h"
#include "Includes/mapper.h"
#include "Includes/configs.h"

int main(void){

	read_values_cfg_port_mapper();
        print_values_cfg_port_mapper(1);
        memset(curr_dir,0,PATHSIZE);
        getcwd(curr_dir,PATHSIZE-1);
        printf("Generalized cfg:\n");
        parse_generalized_cfg(generalized_config_filepath_buff);
        print_values_generalized_cfg(1);

        printf("Curr dir: %s\n", curr_dir);
        
	port_mapper_init(&port_mapper_entry);

        printf("Saimos do port mapper!!!!!!\n");
        return 0;



	return 0;
}
