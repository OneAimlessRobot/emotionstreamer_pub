Um server de musicas. 
Descubram como se usa palhaços. 
Garante-se que funciona em Unix x86 e arm64, 
os executaveis que interessam estão nas pastas server, client, server browser, heartbeat e master_server
quando os argumentos de linha de comandos n servem nos executaveis, diz-vos a sintaxe
O projeto compila com um shell script na root chamado "makeAll.sh" 
Divirtam-se chavalos.


***SÒ FUNCIONA COM FICHEIROS AUDIO WAV (SE NAO ESTAO, CONVERTE!)***


N estraga nada... mas se usarem mp3 e coisas desse genero só dá uma chuva esquisita.

Dependencias para o pacote todo:

- alsa, pulseaudio, ncurses, pthreads, make, sqlite e math.

Como funciona:

Existem 4 partes: Cliente, server, server browser e heart beat server.

O server e o cliente aceitam 4 comandos: 
-play - pedir a transmissao e reproduçao duma musica -down - pedir a transferencia inteira duma musica. Nao toca. 
-conf - pedir a config ao server, para que o cliente possa ajustar a sua de acordo com o que o server tem e com seus objetivos (Sei la... podia 
ter o objetivo de fazer com que n funcione...)
	- A config do cliente esta em code/client/configs/sizes.cfg
	- A config do server esta em code/server/configs/sizes.cfg
-peek - consultar os ficheiros .wav (sem extensao) que o server tem disponiveis.


lançam o server assim:

./(executavel do server) (ip):(porta) (heart


O server cria um fork por cada conexão. (suporta varios clientes em simultaneo)

encerra recebendo um SIGINT ou SIGPIPE.

O server possue uma config no folder "configs" em que definem:

O tamanho de packets transmitidos

Os cliente pode ser lançado de 4 formas:

./(executavel do cliente) 
(request) (nome do ficheiro) 
	
(So para os comandos 'down' e 'play'. Ignorado nos comandos 'peek' e 'conf'. mas teem de meter a mesma. 
	
(Sei la... metam qjhjhjhhjadjakdadlakd ou qualquer coisa do genero...)) 
	
	
(ip server):(porta server)
	
	
o request play tem 2 formas:
	
	
play:(biblioteca de audio) em que <biblioteca de audio> pode ser "alsa" e "pulse". O cliente sai sem fazer nada se n for metido nenhum 
	
dos 2

O cliente sai com um SIGINT ou SIGPIPE, tal como o server.


Os valores cfg_datasize e cfg_chunk_size teem de ser exatamente iguais, porque toda a memoria que o programa aloca, com exepção do buffer de 
video do cliente, na stack.


exemplo config server: (A ORDEM DAS SETTINGS EM CADA FICHEIRO TEM DE SER EXATAMENTE ESTA!)

chunk_size: 1024
server_timeouts_data: 3 5
server_timeouts_con: 3 3
server_timeouts_drop_chunks: 10 3
server_ack_timeout_lim: 3
server_music_folder_path: /songs_in/raw_songs/

exemplo config cliente:

latency_ms: 1000
cache_num_chunks: 3000
cache_almost_full_pct: 70
cache_almost_empty_pct: 15
freq: 44100
client_timeouts_data: 3 5
client_timeouts_con: 3 3
client_ack_timeout_lim: 3
client_music_folder_path: /songs_out/raw_songs_in/
log_file_name: (Podem omitir)



-Make the client of my streamer buffer server sent chunks. I plan on using a queue in circular buffer. There will be 2 cursors; A playing cursor 
and a receiving cursor. When the queue is full, IE it is 1 less unit in the circular sequence than the playing cursor, and it is smaller in that 
same sequence it stops asking for chunks. When both are separated by 1 in the sequence and the playing is bigger, the playback simply stops 
until more chunks come (the recv cursor increases). The client will need to have a circular queue of chunks data structure and 2 threads. The 
queue module will be where the media playing happens at. Acess to the queue will be mutex protected. There will be cond vars. And a latency 
constant that decides how much time is waited until the actual playing begins, to give the queue time to fill up.


The client will also store a file of hosts it previously connected to,so that one can simply pick "last" as the connected address and basically 
iterate them until one works. There will be a stored address data structure that will be gotten rid of as soon as the connection happens.

And, from now on, addresses imputs will be a single <ip>:<port> in the cmdline instead of <ip> AND <port>. its much less clumsy and more memorizable

I will use scanf until I find a colon, take that as ip. the rest is port. Simple! Ill do the same with the server.


The circular queue will be specialized. I dont tend to pay attention to college very much but I know one thing: Its very rare for a generalized 
queue to be efficient and idiot proof. I will just make it, specifically, a chunk queue. You play your chunks and you dequeue and enqueue your 
chunks and check wether or not its full or empty. Thats it. It works with chunks and nothing else. this is important so that I can make it non 
malloc dependent. there will be a balast number: it will be positive when there are chunks to be played. Zero when we played everything. We will 
only go forward.


*back to music streamer*


I got the queue working. Now I should try to milk the f*** out of these 2 concepts: Slide windows and switching buffers. First I will create a 
branch where I copy the stuff from the q_buffer branch to it and then go on and experiment. I shall call the new branch: s_windows. I will try 
to use sliding windows between the rx and play threads in the client and then between the server and the clients rx thread and *just* focus on 
that new concept and keeping the queue. Then... if its faster, I will keep it for the next branch, s_buffer... where I will try to combine 
sliding windows with switching buffers. The main issue with the queue I have now is the fact that multiple threads access the same resource. 
sooo... the locking mutexes make the rx and play threads trip over eachother and the stream ends up slowing down alot... but it is slightly 
faster than before. With switching buffers, Allegedly I only need the mutex to protect the acess to the buffer switching mechanism, where one 
never really has to wait for the other to be done... we just have to switch on use... but then.... how the hell do I combine that with slide 
windows? I dont know.... I dont know and Im tired now... Im gonna leave it be for now... lots of stuff to do...




Ok o streaming ja funciona a 100%. Mas agora quero implementar um server browser.

O sistema vai passar a ter as seguintes componentes:

Server - motor - Conexão (destacada) (fork do engine (vive independentemente do engine))

   -Se o request for down/conf/peek a conexao é só um fornecedor de ficheiro unico. Um source de ficheiros - Thread de acks com o cliente 
   durante a stream. Controla a stream com timeouts. É um mini heart beater - Thread de transferencia. Transfere a musica chunk a chunk.
 - Heartbeat Server interactor. (thread do engine. vive com ele. Assegura que o server se mantem acessivel pelo server browser.)

Cliente: -Faz down/conf/peek - so ele mesmo. É só um sink de ficheiros

 -Faz play

   - Thread de rx - recebe chunks. - Thread de play - toca chunks. Interage com rx por meio de uma queue circular com tamanho definivel por config. - thread de stats- mostra os stats da stream. -Thread de acks do server. É um mini 
   heart beater.



LocalHost SQLite Db de servers: Este componente é uma base de dados gerida localmente no pc onde o Heart beat server é lançado. É atualizada a todo o momento com informação acerca dos servers. Uma tabela de server entries.

Uma server entry é constituida por:

Nome- Criado aleatoriamente na genese do server. String random com tamanho fixo. Chunk size - util para os clientes Ip do server porta do server.


Heartbeat server ja Sei que vai ter dois threads: - Aceitador de pedidos de server browsers para que lhes seja fornecida uma lista de servers. 
(Ok... pensando melhor... nem sei se é possivel transferir assim estas cenas de forma tão direta com simples csvs, fds e sockfds. como estava a 
imaginar... tenho de estudar melhor as bibliotecas de C de SQLite.) - Aceitador de heartbeats. Nao sei: como vai aceitar servers. se ha de fazer 
threads, forks, usar epoll ou select... sou capaz de usar select. Mas vai a todos os socks no seu set de readfds. e vai ter de o fazer rapido. 
Vai ter de se manter a par de quantos heartbeats em atraso tem cada server.

O engine tem uma struct, o streamer cliente tem uma struct, o streamer server tem uma struct Uma conexao é uma struct Um server browser é uma 
struct. Um heartbeat acceptor é uma struct. Um heartbeat connector do server é uma struct. Um chunk player é uma struct.


As portas do server vão ser: Engine: 9001 Conexao: x, x+1, x+2.......: x = 9002 Heartbeater: x, x+1,x+2, x E (1, 31998k) (É este que se conecta, 
logo n precisa de portas fixas.)(random)

As portas do cliente são: x, x+1,x+2, x E (1, 31998k) (random)


Server browser: x, x+1,x+2, x E (1, 31998k) (É este que se conecta, logo n precisa de portas fixas.) (random)

HeartBeat Server:

Aceitador de Heartbeats/fornecedor de lista de servers: x, x-1, x-2.......... x = 8999 & x >=1


Edit: Pera.... mas bases de dados SQLite são... ficheiros Db que basicamente são só csvs glorificados! Eu posso 100% enviá-las a server browsers!

Uma conexão é uma struct que pode ser iniciada de 2 formas: Server: (Nao significa que seja um server. chama-se assim porque a outra versão, a 
client é quem toma a iniciativa. é preciso separar quem toma a iniciativa de quem não toma. Tem 3 sockets, cada uma com seu buffer: j Socket 
tcp, udp e udp_ack. Depois tem um bool is_on e varios metodos de read e send: con_send_udp, con_send_udp, con_send_ack_udp

(resp. read)

Agora... esta completo. O server tem uma config em que la so diz, simplesmente, o ip do heart beat server onde se vai registar, em formato
(ip):(porta)

Pessoas que se liguem a esse heartbeat server usando o browser assim:

./(Executavel do browser) (ip do heart beat server):(porta do heart beat server)

Vao poder ver la o server, se estiver a correr.

Uma server entry tem 4 campos:

(id) (Nome aleatoriamente gerado) (chunk size) (Ip):(porta)


O heart beat server é lançado assim:


./(Executavel do H.B.S.) (ip do heart beat server):(porta do heart beat server)

O server tem de conseguir ver o seu heartbeat server. senão desliga imediatamente. Se o server perder contacto com o heart beat server,
tambem desliga

O diagrama de fluxo é o seguinte:



 Server <-> Heart beat server.

    ^                     ^

    |                     |

    v                     v
    
  cliente           Server browser



Update: Agora temos nodes modulares! Esta tudo explicado na imagem.


Servers sao lançados com:


./server.exe (nosso ip):(nossa porta) (ip dum heartbeat/master):(porta dum heartbeat/master)

./heartbeat.exe (nosso ip):(nossa porta) (ip dum heartbeat/master):(porta dum heartbeat/master)

./master.exe (nosso ip):(nossa porta)

Podem encadealos de acordo com a imagem. (Facilita distribuição. Servers sao como folhas, clientes o ceu, heartbeat servers a madeira e master servers a raiz.

Nada de configs para dar os ips aos programs nem nada dessa parvoice.

E sim. Podem só ter Master servers conectados a servers. Master servers tambem agem como heartbeat servers e partilham componentes.

Isto agora é como uma lista ligada por rede. Os "Pointers" Sao definidos no modulo "interlvl_com.c", em que "Watchdogs" monitorizam "Slaves" em ips diferentes.

Ainda hei de modularizar os threads monitores mas isso é para outra altura.

Para já, temos só isto.

Server browsers podem dar query a um server para perguntar quem é, ou o master (Gajo de cima) De um server, ou quais sao os seus filhos.


Comandos para o server browser fazer isso sao:


./server_browser.exe ('showme' ou 'showme_master') (ip:porta a investigar)

O output apresenta se as entradas que estão abaixo são servers normais (server_mod) ou heartbeat servers (heartbeat_mod)

clientes nao mudaram na operação













