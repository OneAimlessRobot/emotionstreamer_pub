#ifndef CONNECTION_H
#define CONNECTION_H


typedef struct stream_mem{



}stream_mem;

typedef struct connection{

	
        char file_path[PATHSIZE];
	char from_addr_str[INET_ADDRSTRLEN];
        struct sockaddr_in udp_from_addr,
		udp_con_addr;
	
	int fp,is_on,sockfd_udp;
	u_int64_t data_size;
	socklen_t udp_addr_size;
	


}connection;

//file_path is 0 ended!!!!!!
void con_go(int sockfd, uint32_t curr_port,struct sockaddr_in* c_addr,struct sockaddr_in* s_addr);


#endif
