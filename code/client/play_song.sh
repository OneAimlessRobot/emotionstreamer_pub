#!/bin/bash

#get song with pattern:
print_help(){

	echo "Needs 2 args!"
	echo "(num args supplied: $?)"
	echo "1- backend"
	echo "2- expression for song"
	echo "3- > 1 => multiplas tocadas no caso de multiplos resultados de pesquisa"
	echo "4- > 1 => escolher inicio"
}
between_track_wait=3
choose_track_wait=100
startup_wait=0

song_arr=()
at_startup=1
pattern="$2"
backend="$1"
playlist="$3"
choose_start=0
curr_song_index=0
looping=0
count=0

if [ "$#" -lt 3 ];
then
	print_help
	exit

elif [ "$#" -gt 3 ];
then
	choose_start="$4"
fi


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
		echo "Musica numero $i = ${song_arr[$i]}"
	done
}

song_choice_prompt(){
	timeout_song_prompt="$1"
	chosen_index=0
	print_song_dir
	startup_sequence "Choose your option in..:" "$timeout_song_prompt"&
	proc_pid="$!"
	read -t "$timeout_song_prompt" chosen_index
	result_of_read="$?"
	kill -TERM "$proc_pid"
	if [ "$result_of_read" -gt 128 ]
	then
		continue_func
	else
		curr_song_index=$(($chosen_index % $count))
	fi
	echo $curr_song_index
}
continue_func(){

	if [ $at_startup -eq 0 ]
	then
		if [ $looping -eq 0 ]
		then
			echo "Continuing..."
			curr_song_index=$(($curr_song_index + 1))
		else
			echo "In looping mode!"
		fi
	fi
}
repeating_func(){

	if [ $at_startup -eq 0 ]
	then
		echo "Take it back!"
	else
		echo "It behaves like a continuation!"
	fi
}
set_loop_func(){

	if [ $looping -eq 0 ]
	then
		looping=1
		echo "Looping mode enabled!"
	else
		looping=0
		echo "Looping mode disabled!"
		continue_func
	fi
}
exit_func(){

	echo "Exiting as requested!"
	exit

}
pause_prompt(){

	timeout="$1"
	echo "Do you want to leave?"
	echo "type:"
	echo ""
	echo "\"c\" -  continue"
	echo "\"r\" -  repeat last song"
	echo "\"l\" -  toggle loop mode (Automatically repeats last song)"
	echo "\"g\" - go to song"
	echo "\"other\" - leave"
	answer="c"
	startup_sequence "Choose your option in..:" "$timeout"&
	proc_pid="$!"
	read -t "$timeout" answer
	result_of_read="$?"
	kill -TERM "$proc_pid"
	if [ "$result_of_read" -gt 128 ]
	then
		if [ $looping -eq 0 ]
		then
			continue_func
		else
			repeating_func
		fi
	elif [ "$answer" = "c" ];
	then
		continue_func
	elif [ "$answer" = "r" ];
	then
		repeating_func
	elif [ "$answer" = "l" ];
	then
		set_loop_func
	elif [ "$answer" = "g" ];
	then
		song_choice_prompt "${choose_track_wait}"
	else
		exit_func
	fi
}
startup_sequence(){

	prompt="$1"
	countdown="$2"

	echo "${prompt}"

	for(( i=$countdown;i>0 ; i--));
	do
		#echo "$i ..."
		sleep 1
	done

}
ambiguous_request(){

	echo "Foi devolvido mais de um resultado!! Tenta especializar mais na proxima pesquisa!"
	echo "Caso pretendas tocar todas em sequencia,"
	echo "Tenta correr atribuindo \"1\" ao quarto termo argumento a contar do nome do script"

}
play_song_list(){


	if [ $count -gt 1 ]
	then
		startup_sequence "These songs shall be played in sequence in..." "${startup_wait}"
		if [ $choose_start -gt 0 ]
		then
			pause_prompt "${between_track_wait}"
		fi
	fi
	at_startup=0
	for((;$curr_song_index >= 0 && $curr_song_index< $count; ));
	do
		echo "Musica \"${song_arr[$curr_song_index]}\" ira ser tocada!"
		echo "(Numero $curr_song_index)"
		echo "ira ser tocada!"
		./emotionstreamer_client.exe "play:${backend}" "${song_arr[$curr_song_index]}"
		if [ $count -gt 1 ]
		then
			pause_prompt "${between_track_wait}"
		else
			break;
		fi
	done

}
play_single_song(){

	echo "Musica \"${song_arr[0]}\" ira ser tocada!"
		./emotionstreamer_client.exe "play:${backend}" "${song_arr[0]}"
}
play_wrapper_function(){

	if [ $count -lt 1 ]
	then
		echo "Não foram devolvidos resultados do servidor para o padrao fornecido!"

	elif [ $playlist -gt 0 ]
	then
		play_song_list

	elif [ $count -gt 1  ]
	then
		ambiguous_request
	else
		play_song_list
	fi
}
main(){

	result_file="./.tmp_result"

	touch $result_file
	./emotionstreamer_client.exe peek "${pattern}" > $result_file

	fill_up_song_dir
	rm -rf $result_file

	echo "Obtivemos $count resultados de uma pesquisa pelo padrão \"${pattern}\""

	print_song_dir

	play_wrapper_function


}

main
