#!/bin/bash

song_fixing_tmp_file_name_one=".tmp_fixing_songs_brb"
tmp_ext_for_song_in_fixing="mp3_broken"
song_extension="mp3"
boundary_extension="boundary"
array_of_songs=()

if [ "${#}" -ne 1 ];
then
	echo "Usage: arg1: the full path to the directory of songs to fix"
	exit
fi

directory_of_songs="${1}"

pushd "${directory_of_songs}"

find_cmd=$(find . -name "*.${song_extension}" -type f | xargs -I {} basename -s ".${song_extension}" {})

num_of_songs=$( echo "${find_cmd}" | wc -l)

echo "${find_cmd}" > "${song_fixing_tmp_file_name_one}"


echo "We have: $num_of_songs songs in this directory!"


fill_array_of_songs(){
	counter=0
	while read i
	do	
		echo "Counter= ${counter}"
		array_of_songs[$counter]=$i
		counter=$(( counter + 1 ))
		
	done <  "${song_fixing_tmp_file_name_one}"

}

print_array_of_songs(){
	for (( i=0 ; i < num_of_songs; i++ ))
	do
		echo "Song number $(( $i +1 )) = ${array_of_songs[$i]}"
	done
}
#fix_mp3_cmd=ffmpeg -hide_banner -loglevel warning -i 'in.mp3' -ignore_unknown -map 0:a -map_metadata -1 -codec:a copy -bitexact 'out.mp3'
#will receive full dirs!

fix_song_function(){
		song_name="${1}.${song_extension}"
		song_name_two="${1}2.${song_extension}"
		song_name_boundary="${1}.${song_extension}.${boundary_extension}"
		rm -rf "${song_name_boundary}"
		ffmpeg -hide_banner -loglevel warning -i "${song_name}" -ignore_unknown -map 0:a -map_metadata -1 -codec:a copy -bitexact "${song_name_two}"
		rm -rf "${song_name}"
		mv "${song_name_two}" "${song_name}"
	
}
process_songs_loop(){
		
	for (( i=0 ; i < num_of_songs; i++ ))
	do
		fix_song_function "${array_of_songs[$i]}"
	done
	
	
	
}

fill_array_of_songs

print_array_of_songs

process_songs_loop


rm "${song_fixing_tmp_file_name_one}"


popd

find "${directory_of_songs}"


