#include "../../Includes/preprocessor.h"
#include "../Includes/auxfuncs.h"
#include "../Includes/sockio.h"
#include "../Includes/sockio_tcp.h"
#include <openssl/ssl.h>
#include "../Includes/openssl_stuff.h"
#include "../Includes/fileshit.h"


void init_openssl_libs_server_side(void){
	if(logging){
		if(will_use_tls){

			fprintf(logstream,"Server's SSL Ativado? inicializando Server's SSL!...\n");
		}
		else{

			fprintf(logstream,"Server's SSL não ativado. Returning...\n");
			return;
		}
	}

	if(will_use_tls){
		SSL_library_init();
		global_ctx = SSL_CTX_new(TLS_server_method());
		SSL_CTX_set_verify(global_ctx, SSL_VERIFY_PEER, NULL);
		SSL_CTX_load_verify_locations(global_ctx, host_cert_file_path, NULL);
	}

}

void init_openssl_libs_client_side(void){

	if(logging){
		if(will_use_tls){

			fprintf(logstream,"Client's SSL Ativado? inicializando Client's SSL!...\n");
		}
		else{

			fprintf(logstream,"Client's SSL não ativado. Returning...\n");
			return;
		}
	}

	if(will_use_tls){
		SSL_library_init();
		global_ctx = SSL_CTX_new(TLS_client_method());
		SSL_CTX_set_verify(global_ctx, SSL_VERIFY_PEER, NULL);
		SSL_CTX_load_verify_locations(global_ctx, auth_cert_file_path, NULL);
	}


}

void end_openssl_libs_client_side(void){



	if(logging){

		fprintf(logstream,"Closing client's global ssl context?\n");
	}
	if(global_ctx&&will_use_tls){
		if(logging){

			fprintf(logstream,"Yes!\n");
		}
		SSL_CTX_free(global_ctx);
		global_ctx=NULL;
		if(logging){

			fprintf(logstream,"Client's global ssl context closed!!\n");
		}
	}
	else if(logging){

		fprintf(logstream,"Nooooo... (Client's global ctx is null so there is nothing to close)\n");
	}
}

void end_openssl_libs_server_side(void){

	if(logging){

		fprintf(logstream,"Closing server's global ssl context?\n");
	}
	if(global_ctx&&will_use_tls){
		if(logging){

			fprintf(logstream,"Yes!\n");
		}
		SSL_CTX_free(global_ctx);
		global_ctx=NULL;
		if(logging){

			fprintf(logstream,"Server's global ssl context closed!!\n");
		}
	}
	else if(logging){

		fprintf(logstream,"Nooooo... (Server's global ctx is null so there is nothing to close)\n");
	}


}
