#ifndef PORT_MAPPER_FILE_H
#define PORT_MAPPER_FILE_H
#define PORT_MAPPER_FILEPATH "./.port_mapper_prev_conf"

int save_port_mapper_file(port_mapper* mapper);
int fetch_port_mapper_file(port_mapper* mapper);

#endif
