#include "../../Includes/preprocessor.h"
#include <openssl/ssl.h>
#include "../../extra_funcs/Includes/fileshit.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/sockio_tcp.h"
#include "../../extra_funcs/Includes/sock_ops.h"
#include "../../extra_funcs/Includes/ip_cache_file.h"
#include "../../extra_funcs/Includes/connection.h"
#include "../../extra_funcs/Includes/interlvl_com.h"
#include "../../extra_funcs/Includes/generalized_config.h"
#include "../Includes/mapper.h"
#include "../Includes/port_mapper_file.h"
#include "../Includes/configs.h"



int save_port_mapper_file(port_mapper* mapper){

	int port_mapper_local_file_fd=-1;
        if((port_mapper_local_file_fd=open(PORT_MAPPER_FILEPATH,O_WRONLY|O_TRUNC,0777))<0){
                perror("Nao foi possivel abrir ficheiro para truncar!!!! Saindo!!!\n");
		return -1;
        }
	write(port_mapper_local_file_fd,&cfg_num_ports,sizeof(uint16_t));
	write(port_mapper_local_file_fd,&cfg_init_port,sizeof(uint16_t));
	write(port_mapper_local_file_fd,&mapper->curr_num_ports,sizeof(uint16_t));
        write(port_mapper_local_file_fd,mapper->port_arr,sizeof(int32_t)*cfg_num_ports);
        close(port_mapper_local_file_fd);
	fprintf(stdout,"Sucessfully saved array from port mapper file with:\n- %hu declared ports\n- %hu initial port\n- And %hu current stored ports\n",cfg_num_ports,cfg_init_port,mapper->curr_num_ports);
	return 0;

}
int fetch_port_mapper_file(port_mapper* mapper){
	uint16_t local_curr_num_of_ports=0,
			local_curr_num_declared_ports=0,
			local_curr_init_port=0;
        int port_mapper_local_file_fd=-1;
        if((port_mapper_local_file_fd=open(PORT_MAPPER_FILEPATH,O_RDONLY,0777))<0){
                perror("Port mapper file does not exist. Attempting to create...\n");

        }
        close(port_mapper_local_file_fd);
        if((port_mapper_local_file_fd=open(PORT_MAPPER_FILEPATH,O_CREAT|O_RDONLY,0777))<0){
                perror("Attempting to create por mapper file failed. Aborting operation...\n");
		return -1;
        }
	int result0=read(port_mapper_local_file_fd,&local_curr_num_declared_ports,sizeof(uint16_t)),
		result1=read(port_mapper_local_file_fd,&local_curr_init_port,sizeof(uint16_t)),
		result2=read(port_mapper_local_file_fd,&local_curr_num_of_ports,sizeof(uint16_t));
        if((result0<=0)||(result1<=0)||(result2<=0)){

		fprintf(stderr,"Could not fetch array from port mapper file!\nDimension values could not be read...\nError: %s\n",strerror(errno));
		close(port_mapper_local_file_fd);
		return -1;
	}
	int size_matched=(cfg_num_ports==local_curr_num_declared_ports),
		initial_port_matched=(cfg_init_port==local_curr_init_port);
	if(!size_matched||!initial_port_matched){

		fprintf(stderr,"File contains innadequate dimensions for current configuration.\nIgnoring values to create brand new array...\n");
		close(port_mapper_local_file_fd);
		return -1;
	}
	fprintf(stdout,"Sucessfully fetched values for array from port mapper file with:\n- %hu declared ports\n- %hu initial port\n- And %hu currently stored ports\n",local_curr_num_declared_ports,local_curr_init_port,local_curr_num_of_ports);
        mapper->curr_num_ports=local_curr_num_of_ports;
	read(port_mapper_local_file_fd,mapper->port_arr,sizeof(int32_t)*cfg_num_ports);
        close(port_mapper_local_file_fd);
	return 0;
}
