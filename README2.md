```


Other notes:

1- it avoids using malloc as much as humanly possible.

I only intentionally use malloc for the queue buffers in the client
As I meant their sizes to be user modifiable at run time through configs.

2- the threads in the client have names.

The main thread is called main.Beatriz
the decoder is called decoder.Ester
the stats UI thread is called stats.Adriano
the player thread is called player.Filipa
the input thread is called input.Ksun.

These names are written at run time to their COMM strings
on thread startup

You are able to see them in htop if you enable the COMM column.

and is basically what you use to force kill the client if it jams.

If the client jams,
and you dont care about ports being left
erroneously allocated in the port mapper
(Which [will] happen.
Im sorry.
Im already safely terminating using atomic ints,
cleanup functions
and sigaction structs.
but I cannot protect against a KILL signal
if it really is required, okay?)
You do:

username@pcname:/(... some/path/..../...here)$ killall -KILL main.Beatriz [ENTER]

It will then kill the main thread of the client,
which will,
in turn,
force the rest to shut down
with no chance for any masks to catch the signal.

basically:
"I dont care about cleanup.
Kill it."


Anyways.

3- It is very light on resources.

In my macbookpro with a 4 core intel i5 processor in it,
8 gb of ram
and with the OS running on an SSD,

it consumes:
at the configs that were provided:

peak 3% of total CPU capacity
and 60MB of ram

4-  this is important.
So.
If you are only launching the client and port_mapper
to connect to a server,
this is what goes on:
1- the client asks for a port
2- it is given.
3- the client closes the socket
AND returns the port to the port mapper.

But the world is not perfect,
and sometimes,
addresses get stuck in an annoying LAST_ACK state
where there are no CLOSE_WAIT sockets anywhere
But it does not let me use it to connect, for some reason.

So,
if,
with the portmapper still on,
you try to get the same port back that you just returned,
You will have to wait about 5 minutes until "last ack" goes away.

What I suggest you do, in that situation,
is to close the port mapper,

edit the the option in the config
which specifies the range
at which ports start getting served
(which would be the one that you are getting, in this situation,
as you are only using the client,
so that you can connect again more quickly.
Im sorry.
its very awkward and clumsy,
but I dont know another workaround.
Ive tried several ways to close the socket:
using shutdown,
setting the socket's linger to zero
right before closing it...
etc, etc, etc.

It does not work.
You will have to change the port, sometimes.
its frustrating,
So I'm sorry.

I feel rather guilty
for making my system so complicated for people to use.
"What do you mean I need to relaunch your stupid port mapper?"
But,
I need the port mapper.
Sorry...
its how it works
)

NEWS!

AUTOMODE WAS ADDED!

Essentials:
1- its kind of lame,
but the client still needs the parameter to play a song.
BUT
the specific string given is ignored!
2- the server will be given a rotation file
structured like this:
"
time (seconds)
song1.[server_extension]
song2.[server_extension]
....


And at every [time] in seconds+-1 second,
the next song will be chosen in a round robin fashion.

The new server config file looks like this, now:
"

server_logging: 1
server_chunk_size:  1152
server_timeouts_con: 3 3
server_timeouts_data: 3 5
server_timeouts_ack: 3 5
server_ack_period_us: 1000000
server_timeouts_drop_chunks:  3 0
server_music_folder_path: /raw_songs/
server_music_quarantine_folder_path: /quarantine_songs/
server_working_extension: .mp3
server_ip_address: 192.168.1.2:0
upper_server_ip_address: 192.168.1.2:11006
server_port_mapper_ip_address: 192.168.1.2:7777
generalized_config_filepath: /../config/
server_is_auto_mode: 1
rotation_filepath_if_auto: ./configs/rotation.ini
server_name: "ola-bom-dia-como-estas"

"
The server_is_auto_mode parameter enables/disables automode, of course
(0 for off, 1 for on)
And the filepath that follows tells it where to find the rotation configuration file.

A default one could be:

"30
Tijuana_Bass_-_Nortec_Collective__Official_Music_Video_.m3
Resilience.p3
tfaosd/Piedras_vs._tanques.mp3
Stardust.mp3
"

Time is 30 seconds
and all the filepaths are relative to the absolute song path of the server
(
(Server executable pwd)+(server_music_folder_path)+(song_filepath)
)

In this case,
because the working extension is .mp3,
the first two songs would be ignored
and the rotation would only include the last two.

if the file is empty,
the server wont launch
and if only the time is specified
or if it is equal or less to the amount
of server connection waiting timeout seconds plus one,

the server will crash, as well.

Thats it for updates

See you later!


UPDATE!

"rotation" command in the client!

Similar to the "conf" command,
it will print out information related to the server's rotation.
whether it is in auto mode,
the song rotation duration,
Along with the number of songs
which will both be "0"
in case of auto mode not being enabled
and,
if auto mode is enabled,
the song circulation list



thats it for now.
cya!



BUGFIXES:

Fixed a bug where the waited time would only update on full server
timeouts
(for example,
the ~3 seconds of connection waiting period on the config)
which made it so that requests would stall it,
making the song never change.
Now the ending timespec is calculated independently of the result of select
(As it will quit if it is strictly negative anyway
And both positive
and neutral results require a waited time update)

(no mutexes are necessary
as values are "Photographed" between forked processes)


BUGFIXES!!!!!

I FIXED THE PORT MAPPER ISSUE!!!!
I made it so that both:
1- the client
2- the server browser (which now binds to the correct address)

ask for a new port if the first one is jammed!!!
And all the attempted ports are returned at the end!
(hopefully)

FUCK YOU LAST ACK!
YOURE GOD AWFUL






MAJOR BUGFIXES + IMPROVEMENTS!

Made overall code base smaller + more reliable with connection wrapper.
it will attempt several ports until a connection is established.
A compile time constant determines how many ports get delivered per connection attempt.
this was made to go around LAST ACK!
I WON!
I DEFEATED LAST ACK!
Also...
I made port mapper interactions consume less bandwidth.
Ports get delivered all at once,
directly delivered in a counted array.
The first slot defines the number of ports to send/return
the remainder is consecutive ports.
Its an improvement over the previous implementation
Where I would actually parse and build strings using sscanf and snprintf
Where they would be formatted like:
"number port1 port2 ... port[number]".
In fully spelled out strings.
As in,
the digits were characters
not integer values.
Which is,
very bad.

Ive added a protection that prevents some ports
from being freed when they should not,
by zeroing out the array once its done.

the multiple port attempt circuit only needs to work once,
so,
once ports are given,
memmoved so the winning port is kept,
then the array can just be delivered,
zeroed out,
after which the used port gets delivered on its own,
also with a binary protocol.


There are protocol specific strings I use to let the port mapper know
that it is one of my services who is talking to it.
So proper strings are required in that case.

As well as in other interconnected modules.


Also,
Ive added a splash screen!
yeey...
All modules,
with the exception of the converter tool,
will display a splash ascii art file at start up.

It is located in the config folder
in the code directory of this repository
And it can be anything you want.
Nothing will really happen if the file fails to be opened.
And it will just get printed as is
With the slight addon of a white background
and blue colored font.


Ive also converted the peek request handler in the server
from a system() setup,
Which was horrible,
and I feel horrible for ever using it,
with an execvp alternative.

I avoided using execvp
because I wanted that part of the code
to be only made of parts I understood.

I do now understand how dup2 works,
after a back and forth with chatgpt
and what basically goes on is:

dup2 takes two file descriptor numbers.
each originally refering to two devices.
lets say number x refers to file device A
and number y refers to file device B.

the duplication call I perform in the childbranch of the fork
is this one:

dup2(x,y);

dup2 will make it so that:
the number y will now point to device A
As well as number x.

After that,
I did not understand why close(x) was necessary
but it is due to the fact
that we no longer need x for this device.
So we can just close it so that we dont have a file descriptor leak.

Also,
We do this because in the following execvp call,
we will redirect its output to file number y,
and therefore device A.

And that is how it works, from my understanding.
There.
I think I am allowed to use it, now.

Ok.
I might also separate this readme into several parts.
As these notes become too much text on their own.

NOTE:
I split the readme in two parts, now!
README1.md and README2.md

UPDATE:

Ive added several compile time constant uint8_t's
in the configs header of each module
which determines whether to display the splash screen at startup!
By default,
the port mapper wont display it, for example.
But the master
and content servers will.

As a reminder:
to compile the program,
Make sure the dependencies mentioned in README1 are met,
(Also,
Open an issue if you are confused about those)
And head to the code directory,
and execute:
"bash makeAll.sh"
it will attempt to compile every single module,
If you do not modify it.

You can also select which are the one you want,
by removing or adding according to your needs.

In this shipment,
everything is attempted to be built.

But you can change that.


``
