#!/bin/bash

##fix_mp3_cmd=ffmpeg -hide_banner -loglevel warning -i 'in.mp3' -ignore_unknown -map 0:a -map_metadata -1 -codec:a copy -bitexact 'out.mp3'
boundary_extension=".boundary"
mp3_extension=".mp3"
song_directory="../../../raw_songs/"
song_subdirectory="Kichi_-_REBORN/"
expression_to_search="*"
song_basenames=()
num_of_songs=0

for filename in $(ls ${song_directory}${song_subdirectory}*${expression_to_search}*${mp3_extension})
do
	if [ -f $filename$boundary_extension ];
	then
    		echo "File ${filename} already_compiled! Not adding!!"
	elif [ -f $filename ];
	then
		song_basenames[$num_of_songs]=$(basename $filename);
		let num_of_songs=num_of_songs+1;
		echo $num_of_songs;
		echo $filename;
	fi
done

for ((i=0;i<num_of_songs;i++));
do
	echo ${song_basenames[i]};

done



for ((i=0;i<num_of_songs;i++));
do
    ./converter_tool.exe "${song_subdirectory}${song_basenames[i]}" "${song_subdirectory}${song_basenames[i]}"
done

echo "Done compiling all the songs!";
