#!/bin/bash

#get song with pattern:

pattern="$1"

result_file="./.tmp_result"
touch $result_file
./client.exe peek "${pattern}" > $result_file

cat $result_file

num_results=$(cat $result_file| wc -l)
result=$(cat $result_file)
rm -rf $result_file

echo "Obtivemos $num_results da pesquisa pelo padrão: '$1'"

backend="oss"

if [ $num_results -lt 1 ]
then

	echo "Não foram devolvidos resultados do servidor para o padrao fornecido!"
elif [ $num_results -eq 1 ]
then

	echo "Musica \"${result}\" ira ser tocada!"
	./client.exe play:${backend} ${result}
elif [ $num_results -gt 1 ]
then

	echo "Foi devolvido mais de um resultado!! Tenta especializar mais na proxima pesquisa!"
fi
