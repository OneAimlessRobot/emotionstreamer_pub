#!/bin/bash

#get song with pattern:

exit_wait=3
print_help(){

	echo "Needs 2 args!"
	echo "(num args supplied: $?)"
	echo "1- backend"
	echo "2- expression for song"
	echo "3- > 0 => multiplas tocadas no caso de multiplos resultados de pesquisa"
}

fill_up_song_dir(){
	while read -r the_song
	do
		song_arr[count]="${the_song}"
		#echo "Musica \"${song_arr[$count]}\" agora esta no array!"
		((count++))
	done < $result_file
}

print_song_dir(){
	for((i=0; i<$count; i++));
	do
		echo "${song_arr[$i]}"
	done
}
exit_prompt(){

	timeout="$1"
	echo "Do you want to leave?"
	echo "type anything different from \"0\" to leave"
	echo "And \"0\" to continue"
	answer="1"
	startup_sequence "Choose your option in..:" "$timeout" &
	proc_pid="$!"
	read -t "$timeout" answer
	result_of_read="$?"
	kill -TERM "$proc_pid"
	if [ "$result_of_read" -gt 128 ]
	then
		echo "Continuing"

	elif [ "$answer" = "0" ];
	then
		echo "Continuing!"

	else
		echo "Exiting as requested!"
		exit
	fi
}
startup_sequence(){

	prompt="$1"
	countdown="$2"

	echo "${prompt}"

	for(( i=$countdown;i>0 ; i--));
	do
		echo "$i ..."
		sleep 1
	done

}
play_wrapper_function(){

	if [ $count -lt 1 ]
	then

		echo "Não foram devolvidos resultados do servidor para o padrao fornecido!"

	elif [ $playlist -gt 0 ]
	then
		startup_sequence "Uma lista de ${count} musicas foi carregada para a memoria e tocadas em sequência em..." 5

		for((i=0; i< $count; i++));
		do
			echo "Musica \"${song_arr[$i]}\" ira ser tocada!"
			./emotionstreamer_client.exe play:${backend} ${song_arr[$i]}
			exit_prompt "${exit_wait}"
		done
	elif [ $count -gt 1  ]
	then

		echo "Foi devolvido mais de um resultado!! Tenta especializar mais na proxima pesquisa!"
		echo "Caso pretendas tocar todas em sequencia,"
		echo "Tenta correr atribuindo \"1\" ao quarto termo argumento a contar do nome do script"
	else
		echo "Musica \"${song_arr[0]}\" ira ser tocada!"
		./emotionstreamer_client.exe play:${backend} ${song_arr[0]}
	fi
}
main(){

	result_file="./.tmp_result"

	touch $result_file
	./emotionstreamer_client.exe peek "${pattern}" > $result_file
	count=0

	fill_up_song_dir

	print_song_dir

	echo "Obtivemos $count da pesquisa pelo padrão: '$2'"

	play_wrapper_function

	rm -rf $result_file

}
if [ "$#" -ne 3 ];
then
	print_help
	exit
fi

song_arr=()

pattern="$2"
backend="$1"
playlist="$3"

main
