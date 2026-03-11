#!/bin/bash

#get song with pattern:

if [ "$#" -ne 2 ];
then
	echo "Needs 2 args!"
	echo "(num args supplied: $?)"
	echo "1- backend"
	echo "2- expression for song"
	exit
fi

pattern="$2"
backend="$1"
result_file="./.tmp_result"
touch $result_file
./emotionstreamer_client.exe peek "${pattern}" > $result_file

cat $result_file

num_results=$(cat $result_file| wc -l)
result=$(cat $result_file)
rm -rf $result_file

echo "Obtivemos $num_results da pesquisa pelo padrão: '$2'"


if [ $num_results -lt 1 ]
then

	echo "Não foram devolvidos resultados do servidor para o padrao fornecido!"
elif [ $num_results -eq 1 ]
then

	echo "Musica \"${result}\" ira ser tocada!"
	./emotionstreamer_client.exe play:${backend} ${result}
elif [ $num_results -gt 1 ]
then

	echo "Foi devolvido mais de um resultado!! Tenta especializar mais na proxima pesquisa!"
fi
