Um server de musicas. 
Descubram como se usa palhaços. 
Garante-se que funciona em Unix x86 e arm64, 
os executaveis que interessam estão nas pastas server e client
quando os argumentos de linha de comandos n servem nos executaveis, diz-vos a sintaxe
O projeto compila com um shell script na root chamado "makeAll.sh" 
Divirtam-se chavalos.
BTW os requests do client são "play" e "peek". (CASE SENSITIVE)
As musicas que os server emite ficam no folder "songs_out" na root do mesmo.
Existe uma config no folder "configs", onde tambem podem mudar onde o server vai buscar as musicas.



***SÒ FUNCIONA COM FICHEIROS AUDIO WAV (SE NAO ESTAO, CONVERTE!)***


N estraga nada... mas se usarem mp3 e coisas desse genero só dá uma chuva esquisita.

P threads também.

Optimizei config default.

Funciona com pulseaudio ou com ALSA. (Um deles tem que estar instalado.)

O server compila sem as bibliotecas de audio! (Usa 'make cleanall && make server' para compilares Só o server)
