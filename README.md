EmotionStreamer.

This is meant to be a personal project
With a somewhat straightforward process of installing.

It has these dependencies:
1- mpg123 (it comes with a statically assembled library (x86)) (decoding library for mp3 mode)
2- sqlite3 (it comes with a statically assembled library (x86)) (for storing temporary server lists.
Used by the heartbeat, master_server and server_browser modules

3- the same thing for libasound.
4- It does depend on libao
but I could not get it to function.
It does compile,
using a dynamically linked library
but I cannot get it to play no matter what I do.
5- it depends on osspd as well
6- pulse audio and pulse audio simple.

So,
the list of libraries you actually [must] install is the following:
- libpulse-dev
- libao-dev (plugins ought to come included)
- pthreads (ought to come with installation.
if not,
contact me
and we will help you sort it out.
Publish an issue)


Now.

The way it works is:

Port mapper:


1- There is a port mapper module
with the following config:
"
port_mapper_logging: 0
port_mapper_init_port: 11006
port_mapper_num_ports: 5000
port_mapper_ip_address: 192.168.1.2:7777
port_mapper_generalized_config_path: /../config/
"

the first option is 1 or 0
Enable or disable logging.

two:
the second option is the port range it starts serving at.

three: the number of ports after the initial port that it serves.
in this case,
it would be from 11006 to 16006.

three: the ip it will bind to
for other modules to connect
and fourth:
the generalized config path.
which will be used to fetch the timeouts
for port mapper connections.


the port mapper acts as a hotel room booker agent.
Everyone gets permission to use ports from it.

it gives the service that connects to it a port,
and that same service has to return those said ports
in order for others to use them.

There are 2 commands available
to input as characters.
1- 'p' + enter -> it displays the current layout of ports.
reserved,
allocated
and free.
As well as the two next ports that would be returned in the following connections
2- 's' + enter-> exit the port mapper.

Warning: Notifying the connected modules
that the port mapper is dead is not implemented!
So,
If you are hosting this,
It is advisable for you to kill any lingering services first
before shutting it down

2- Master server /Heartbeat server + server master ->

they have the following configs, respectively:

"master_server_logging: 1
master_timeouts_con: 3 3
master_timeouts_data: 3 5
master_timeouts_ack: 3 5
master_ack_period_us: 1000000
master_server_ip_address: 192.168.1.2:0
master_server_port_mapper_ip_address: 192.168.1.2:7777
generalized_config_filepath: /../config/

"

"hb_server_logging: 0
hb_timeouts_con: 3 3
hb_timeouts_data: 3 5
hb_timeouts_ack: 3 5
hb_ack_period_us: 1000000
heartbeat_ip_address: localhost:0
upper_server_ip_address: localhost:11000
heartbeat_port_mapper_ip_address: localhost:7777
generalized_config_filepath: /../config/
hb_server_name: "Estou-bem-e-tu?"
"


"browser_logging: 1
browser_timeouts_con: 9 9
browser_timeouts_data: 9 9
server_browser_port_mapper_ip_address: 192.168.1.2:7777
server_browser_ip_address: 192.168.1.2:0
generalized_config_path: /../config/
"


The master server is the first to start after the port mapper.
It allows  both servers (serve content) and heartbeat servers (intermediate monitors)
to connect to it directly
And to,
having access to the port mapper's ip address,
(which they always must),
bind to their respective addresses
filling the '0' port slots
with a port given to them by the mapper
within its range.

the timeouts are used in select calls
the first parameter being seconds
and the second being microseconds,
(the actual times are not important,
just remember to not pick anything absurd.
like something really short.)
to wait for packets to be sent or received by either member of the connection.


heartbeat servers need a master to connect to,
where the master acts as an index that is searchable
throught the server browser.

the server browser takes two things as parameters
apart from the program name:

1- command ("showme" or "showme_master")
2- an ip address (expressed via <ip>:<port>)
that corresponds to the ip address of the service they want to query.

the query can have these kinds of results:
1- 

"------------------CABEÇALHO DA LISTA DE DOWNS DESTE NODO!!!!!!-----------------------
                                                                                               
[ MODULE_TYPE | id | MODULE_NAME | MODULE_ADDR | FILE_EXT ]

[ server_mod | 5 | ola-bom-dia-como-estas | 192.168.1.2:11007 | .mp3 ]
done

"

(here is the same while a heartbeat server is also connected to that same master)

"------------------CABEÇALHO DA LISTA DE DOWNS DESTE NODO!!!!!!-----------------------

[ MODULE_TYPE | id | MODULE_NAME | MODULE_ADDR | FILE_EXT ]

[ server_mod | 5 | ola-bom-dia-como-estas | 192.168.1.2:11007 | .mp3 ]
[ heartbeat_mod | 6 | Estou-bem-e-tu? | 192.168.1.2:11013 | N/A ]
done
")

this is the result of sending a "showme" command to either a master server
or a heartbeat server.

ads we can see,
this module has a content server logged into it,
serving mp3's at the ip: 192.168.1.2:11007.
Modules have customizable names,
Ids
And they can be:
1- server_mod
2- heartbeat_mod (or hb_mod. I dont remember that well, honestly).
And master servers dont need to connect to anyone
as they are the root.
so they dont need a module_type.

2-
"Sup. Im master. Waddyawant?

done

"

this is the result of sending "showme_master" to a master server.
as they have no parent in the tree,
they send back this rather humurous reply to indicate that.

3-
"Nao sou um master.Mas, se quiseres, Está aqui o meu master.Tenta falar com ele: 192.168.1.2:11006

done
"

this is the result of sending a "showme_master" to a heartbeat server.

and the purpose of these modules,
is to create a similar server browser experience as found in games like:
Counter strike
Quake
Etc.

because I thought it would be cool.


They all connect through a protocol where they connect with tcp,
not before deploying watchdog/ acceptor threads 
(in the case of heartbeat and master servers)
And slave threads (in the case of heartbeat and content servers).

These slave and watchdog threads communicate with each other
and,
if one heartbeat fails,
then the one who pulls the plug will be removed from the connection.

Basically,
if the master server dies,
everyone below it dies,
the same is for a heartbeat server.
but the modules above it,
(In the case for a content server or a heartbeat server)
stay alive
ready for more connections.

This entire system uses solely TCP as a transport layer.
its better than UDP because UDP is unstable,
requires too much NAT gymnastics to get to work on public networks
and is overall a pain in the ass.

So,
some commits ago,
I ditched UDP entirely,
as it,
well...
sucked for what I wanted to do.

Anyways.

Now.

I also have other modules.
And these are more intricate and more complex to explain.
So I will begin with the simplest one:

1- the converter tool.

it takes two directories as arguments in the config:
an input and output.

the objective of this tool
is to create a binary file comprised of the structs
that the server will read,
when in mp3 mode
in order to deliver accurate mp3 frames to the client.

this is the layout of a frame_info_struct:

"
typedef struct frame_info_t{

	uint64_t frame_id,
		start;
	uint16_t size;
	uint8_t	mpeg_layer,
		mpeg_version,
		mpeg_bitrate_idx,
		mpeg_sample_rate_idx,
		mpeg_padding;
	uint16_t mpeg_samples;
	uint8_t	 mpeg_slotsize;

	uint32_t sample_rate,
		 bitrate;

}frame_info_t;
"

this stuff is encoded into ".boundary" files
which will be what the server actually looks for when asked for an mp3 file.
but.
if either the mp3 file
or boundary file are not present,
nothing will be played
and the stream connection will be killed.

They are sent,
along with the actual bounded mp3 data
to the client
(When in mp3 mode)
and in order.

(that frame_id field saved me multiple hours of pain while debugging,
by the way.
So,
if I had to give any personal advice,
it would be to,
well...
always do it, I guess.).

These basically do not matter in wav mode
and chunks are always sent as they are to the client
which will not attempt to perform any treatment to them whatsoever,
as it assumes the wav is well behaved.

but,
clients can report wav files
(and mp3 files alike)
with a report command,
so that is how I deal with faulty
or malicious files.


the converter takes an input mp3 filename from the input directory
and an output boundary filename in the output directory.
directories are not created if they dont exist.
And both files have to have the same non-extension base name
in order for the system to work.
I know.
its silly,
but yeah.

Maybe I could insert a filename line at the beginning
in order for the server to know which mp3 file
we are actually talking about,
but oh well.

Anyways.
A boundary file is created
which will then have to be placed in the same directory
as the mp3 file itself

So now,
I think it is a good time to get to the content server
the heart of this whole system:


The content server has the following style of config:

"server_logging: 1
server_chunk_size:  1152
server_timeouts_con: 3 3
server_timeouts_data: 3 5
server_timeouts_ack: 3 5
server_ack_period_us: 1000000
server_timeouts_drop_chunks:  3 0
server_music_folder_path: /raw_songs/
server_music_upload_folder_path: /raw_songs/
server_music_quarantine_folder_path: /quarantine_songs/
server_working_extension: .mp3
server_ip_address: 192.168.1.2:0
upper_server_ip_address: 192.168.1.2:11006
server_port_mapper_ip_address: 192.168.1.2:7777
generalized_config_filepath: /../config/
server_name: "ola-bom-dia-como-estas"
"

(by the way:
Just a heads up.
These configs are [extremely] sensitive.
They have to always have those fields
by that order.

the values change, of course.
but try not to mess witht he format.
timeouts are always two integers,
single numbered parameters are always a single number,
and strings are,
well,
strings.

Ips are <ip>:<port>
...)


the upper server address is the master/heartbeat server it connects to.
(The same parameter exists in heartbeat servers.
And yes.
heartbeat servers [can], indeed, connect to other heartbeat servers.
Its how I plan to deal with connection distance.


the server chunk size is...
a tricky parameter.

for wav files,
pretty much anything that isnt absurd,
(like anything below zero or above 65355 specifcially)
Any size goes.
But.
For .mp3 mode 
(triggered by launching the server with ".mp3" as an extension (without the quotes))
1152 is the size to use.
Always.

And...
if the client plans on connecting to an mp3 server,
the size to use is [always] 4608.
because ive found that
anything above it will cause distortions.
And anything below it will make the song not play all the way.
As the stream will cut itself short.

but if connecting to a wav server,
anything goes, once again.
although its recommended to match the number.
But feel free to experiemnt
and give feedback if anything goes wrong.


the ack period parameters are used in the slave/watchdog interactions
as to control cpu usage.

the server has paths to which "quarantined" files
(reported by report commands)
uploaded files
and served files are.

The repo comes with some symbolic links
which you can use to create directories outside of the source code folder
and not lug gigabites of audio files around at every commit.
Learn symlinks guys.
they are very useful.

Look up if you dont know.

Anyways.

The content-server pretty much only supports serving
(And the client only supports playing)
two kinds of files:
1- wav files
2- and mp3 files.

A boolean is triggered by either of those strings
(why is the configuration value [not] a boolean by itself?
Well,
I guess that can be the next refinement I make.)



Now.
the last one is the client.
Which is,
I think,
the hardest to explain.

And comes
with the following config:

(line numbers are there for the sake of explanation.
But not in the actual config)

"
1 client_logging: 0
2 stream_enable_ncurses: 0
3 stream_show_stats: 0
4 stream_show_frames: 0
5 ui_framerate_fps: 20.0
6 latency_ms: 1000
7 decoder_cache_num_chunks: 100
8 player_cache_num_chunks: 100
9 cache_almost_full_pct: 60
10 cache_almost_empty_pct: 3
11 show_decoder_queue_length: 200
12 show_player_queue_length: 200
13 show_decoder_queue: 1
14 show_player_queue: 1
15 client_chunk_size: 4608
16 client_timeouts_con: 3 3
17 client_timeouts_data: 1 222000
18 client_music_folder_path: /songs_out/raw_songs_in/
19 client_music_upload_folder_path: /songs_out/raw_songs_in/
20 log_file_name:
21 client_device_name_if_alsa: plughw:1
22 client_device_output_if_alsa: 0 
23 client_alsa_device_latency_if_alsa_ms: 200  
24 server_ip_address: 192.168.1.2:11007
25 client_ip_address: 192.168.1.2:0
26 client_port_mapper_ip_address: 192.168.1.2:7777
27 generalized_config_filepath: /../config/
"


Ok.
Wow.

Im so sorry, guys
I know that it is a lot.

but the first 5 options,
and the ones from 11 to 14
Are purely cosmetic,
So we will skip those for now.

Now.

options 24 to 26 are addresses.
They are self explanatory:
24- which server do I want?
25- which address to I bind to?
26- what address is the port mapper I want to connect to?


the chunk size is recommended to stay at that value for mp3 mode.
the timeouts are self explanatory
"what is the tolerance for connection latency?
what is the tolerance for data latency?"
then,
we have,
respectively,
the output folder for songs which are downloaded,
and the input folder from which songs are uploaded to the server.

the "almost full/empty" parameters control cpu usage.
(its better to only fill up the player when it gets below a certain percentage
instead of ramming things into it constantly
which would make cpu usage higher)

Latency is the time the client waits,
hardcoded,
before starting to play.
its useful to let it "warm up" in case connection/transfer speed is slow.


the number of chunks in each queue is useful when you want to optimize
between memory usage
and connection speed.
It can be as big as you want
but beware.
if the almost full pct of a buffer is larger than the song,
the song wont play!


the cosmetics are options like,
enabling ncurses (for the upload, download and,
of course
the play commands)

for wav mode,
no decoder exists.
so any parameters related to the decoder wont matter
including cosmetic ones.

The client has these commands:

1- play

the play command is invoked as follows:

./client.exe play:<backend> "filename".

where backend can be one of the following [exact] strings:

1- oss:
play using the OpenSoundSystem library.
basically just ram sound into your sound card.
raw af ngl.
Because it depends on the osspd daemon,
some systems may not have it in their repositories
and therefore will not work)
2- alsa:
play using ALSA.

the codebase [does] come with a precompiled,
static ALSA backend.
But make sure to compile for your system.
just in case....
3- pulse:
Play using a PulseAudio server.
Highest level of compatibility.

Play using a connection to a pulse audio server
in case your machine has one running.
4- ao:
Play using the AudioOutput library.
Okay.
Now.

I could not get this one to work.
I simply could not!

I tried everything:
Installing plugins,
uninstalling them,
tried to link them statically,
dynamically...
but even if the plugin appears as registered,
the thing just.
will not.
play.



Alright.
The next commands are a bit more friendly.

2- upload.
it takes a filename
relative to the upload directory.

(also.
the client in this codebase also comes with symbolic links.
Feel free to erase them and make your own if you wish to do so)

Now.
this one may be a bit dangerous.
As the server does not really...
filter what files can be uploaded to it, at the moment.
not even by extension.
But it is convenient in this early stage of this project.
(its been a year...
"early"

But ive also been the sole developer
so I have an excuse to take a long time, okay? >[] )

3- report.

What this command simply does is:

it sends an instruction for the server to remove a .wav/.mp3 file
from the circulation.
.wavs take a bit longer as they are,
by far,
way heavier than the .boundary files that get removed while the server
is running on mp3 mode.

it takes a file_name as argument.
with full path.

Also:
for the download and upload commands,
at either side,
directories get recursively created if they dont exist.

4- conf.

simple.
it fetches the configuration file from the server.

(it also requires a useless second parameter.
it can be just two quotes.
Its just that the whole program only runs commands with two parameters.)


5- peek.

this command takes a second argument
which can be a regular expression
and what it does
is fetch a list of songs in circulation
along with their directory names
relative to the server's music folder
and send it back to the client.

for all the songs,
one can just input two quotes.


expect to use this command quite often.
its really useful



And...
that is about it!

Oh!
I almost forgot a minor thing.

the generalized config simply has the following contents

"port_mapper_timeouts_con: 1 500000"


and is stored in the configs directory
at the "code" folder of this repository

logic for its parsing
is in the extra_funcs directory.


Also.
every module excluding the converter tool,
server browser
and client
(so,
basically all the "servers")

take no parameters for their execution.
They can be executed simply by invoking their executable name
preceeded by a dot-slash (./)

And all configs for specific modules
Are all located in "config/size.cfg" files in the roots of all of them.

For the content server,
the config file is at,
starting from "code",
"server/configs/sizes.cfg"

And for:
- client,
- heartbeat,
- converter_tool,
- browser,
- master_server,

just replace "server" with those names and you will find them.
theya are always called "sizes.cfg"

the general config is called,
well...
"general_config.cfg"
and is located,
starting from the code folder,
in the "config" directory.

Why is it "config" and not "configs"?
Well,
I guess I could change that, then...

Oh.

A small note:
I wont change type for the extension for the server config.

Its useful.
I can use it to print it directly into the databases kept by the master
and heartbeat servers
and shown directly by the queries from the server_browser


Well,

its been one year of work.

but its here.
And it works.

Oh!
and the compilation command is contained in the makeAll.sh shell script!

Anyways, BYEEE!!!!


Oh.
And you absolutely can just use "localhost" if you mean to use this locally.
everything here works locally.
You could literally use it as a personal mp3 player if you so desired.

First:
You set the port mapper address
in its config
to "localhost".
the port is whatever is available.
anything from 0 to 65535 will do.

second,
you launch a master_server.
it must also be in localhost,
as the port mapper also is.

it will launch.
pay attention to the port it got from the port mapper.
it will display repeatedly along with its address in a loop.

then,
you can just skip the heartbeat server.
as those are only meant for connection speed management.

you set the server upper address 

in its config
to "localhost:<master_server_port>"
(or "127.0.0.1:<master_server_port>" if you really want to)

the port mapper address will be the same as the one from the master
and the bound address is,
well...
localhost!

Then
you launch it with no command line parameters
(Remember that the master server and port mapper
also do not need command line parameters) 


And it will periodically print a line containing its address as well.
Then,
you do the same steps with the client,
port mapper and all,

and then you're ready to launch some commands!

Oh.
Just a heads up.

the port mapper is always required to be on.
it is required to be on when launching to a remote server
and locally, too.

but if you're just connecting
and not hosting,
that is all you will need.

After the configs are set,
these are the steps you should follow
to host 
and play in localhost:

1- change directory to code/port_mapper
2- ./port_mapper.exe
3- change to code/master_server
4- ./master.exe
5- ... to code/server
6- ./server.exe
7- change to the client folder
8- ./client.exe <client command> <parameter>


If you only want to connect remotely,
make sure to set the server address adequately,
and bind both the client
and port mappers to addresses which can see the server.

So,
after setting the configs,
in that case you simply do:

1- repeat steps one and two in the previous step list
2- skip and do steps 7 and 8.


And you're done!

If you have any issues or doubts,
feel free to create an issue
and Ill make sure to reply!

Cya!
