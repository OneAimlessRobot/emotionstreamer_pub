#ifndef MORE_SOCKET_OPS_H
#define MORE_SOCKET_OPS_H



void set_sock_os_keepalive(int* socket, int on_or_off);

void set_tcp_socket_keep_idle(int * socket, int secs);

void set_tcp_socket_keep_itvl(int* socket, int secs_period);

void set_tcp_socket_keep_cnt(int* socket, int times);

void create_safety_pipe(int safety_pipe[2],char* pipe_desc,char* module_desc,int flags);

void set_sock_reuseaddr(int*socket,int on_off);

void set_sock_sendtimeout(int*socket,int timeout_s,int timeout_us);

void set_sock_recvtimeout(int*socket,int timeout_s,int timeout_us);

void setSocketRecvBuffSize(int*sd,int size);

void setSocketSendBuffSize(int* sd,int size);

int getSocketRecvBuffSize(int*sd);

int get_sockerr(int*sd);

int getSocketSendBuffSize(int*sd);

void setLinger(int*socket,int onoff,int time);

void socket_close(int* fd, int right_now);

int same_addr_sock_rebind(int*sd);
#endif
