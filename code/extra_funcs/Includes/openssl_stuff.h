#ifndef OPENSSL_STUFF
#define OPENSSL_STUFF

void init_openssl_libs_client_side(const char* cert_file_path);

void init_openssl_libs_server_side(const char* cert_file_path);

void end_openssl_libs_client_side(void);

void end_openssl_libs_server_side(void);

#endif
