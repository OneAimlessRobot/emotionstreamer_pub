#ifndef CONNECTION_H
#define CONNECTION_H


typedef struct stream_mem{



}stream_mem;

typedef struct connection{

	
        char file_path[PATHSIZE];
	char from_addr_str[INET_ADDRSTRLEN];
        struct sockaddr_in tcp_from_addr;
	int fp,is_on,sockfd_tcp;
	u_int64_t data_size;


}connection;

//file_path is 0 ended!!!!!!
void con_go(int sockfd,struct sockaddr_in* c_addr);


#endif
