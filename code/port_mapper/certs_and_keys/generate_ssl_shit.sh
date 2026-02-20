#!/bin/bash
module_name_file=".module_name"

module_name=$(cat "$module_name_file")

echo "Module name: $module_name"

name_string_for_authority_stuff="ca-emo_mer"
name_string_for_server_string="${name_string_for_authority_stuff}_${module_name}"
	
gener8_auth_cert(){

	openssl genrsa -out "${name_string_for_authority_stuff}.key" 4096
	 
	openssl req -x509 -new -nodes -key "${name_string_for_authority_stuff}.key" -out "${name_string_for_authority_stuff}.crt" -days 3650 -config "${name_string_for_authority_stuff}.conf"

}

gener8_module_cert(){
	openssl genrsa -out "${name_string_for_server_string}.key" 2048

	openssl req -new -key "${name_string_for_server_string}.key" -out "${name_string_for_server_string}.csr" -config "${name_string_for_server_string}.conf"


	openssl x509 -req -in "${name_string_for_server_string}.csr" -CA "${name_string_for_authority_stuff}.crt" -CAkey "${name_string_for_authority_stuff}.key" -CAcreateserial -out "${name_string_for_server_string}.crt" -days 825 -sha256 -extensions v3_req -extfile "${name_string_for_server_string}.conf"
}

gener8_module_cert
