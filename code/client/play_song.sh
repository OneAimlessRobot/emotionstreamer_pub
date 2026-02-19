#!/bin/bash

#get song with pattern:

pattern="pretty"

result=$(./client.exe peek "${pattern}" | grep "${pattern}")

print_command="printf \"${result}\n\""

$print_command

num_results=$($print_command| wc -l)

echo $num_results


backend="oss"

if [ $num_results -eq 1 ]
then
	echo "Musica \"${result}\" ira ser tocada!"

	./client.exe play:${backend} ${result}
elif [ $num_results -lt 1 ]
then

	echo "Não foram devolvidos resultados do servidor para o padrao fornecido!"
elif [ $num_results -gt 1 ]
then

	echo "Foi devolvido mais de um resultado!! Tenta especializar mais na proxima pesquisa!"
fi
