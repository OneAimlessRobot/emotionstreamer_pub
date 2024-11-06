#include "../../Includes/preprocessor.h"
#include "../Includes/menu.h"

typedef char* string;

static string menu[]={"Os comandos de operacao sao estes:\n",
	"G - pausar/tocar",
	"S - sair",
	"T - ver temp restante",
	"I - ver info da musica",
	"R - reiniciar",
	">: ",
	"\n",
	NULL};

void print_menu(void){


	for(int i=0;menu[i];i++){

		printf("%s\n",menu[i]);
	}


}

static int filter_char(char c){

	if(!isalnum(c)||!isspace(c)){
		return 1;
	}
	return 0;



}

char receive_cmd(char c){

	if(filter_char(c)){


		if(!isspace(c)){

			return tolower(c);

		}
		return c;
	}
	return 0;



}
