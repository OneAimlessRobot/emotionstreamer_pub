#ifndef OPENSSL_STUFF
#define OPENSSL_STUFF

void InitializeSSL(void);

void DestroySSL(void);

void ShutdownSSL(SSL** cSSL);

void convert_fd_to_ssl(SSL* cSSL, int sd);

void init_openssl_libs_client_side(void);

void init_openssl_libs_server_side(void);

void end_openssl_libs_client_side(void);

void end_openssl_libs_server_side(void);

#endif
