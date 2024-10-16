BINARY= emotionserver.exe

SERVERBIN= ./server/server.exe

CLIENTBIN= ./client/client.exe

CLIENTBIN_NOSDL= ./client_no_sdl/client.exe

GLDIR= ./player

GLIBS := -L$(GLDIR)/SDL2/lib $(GLDIR)/SDL2/lib/libSDL2.a  $(GLDIR)/SDL2/lib/libSDL2_mixer.a -lm -ldl -lpthread
GLIBS += -I$(GLDIR)/SDL2/include -Wl,-rpath,$(GLDIR)/SDL2/lib -Wl,--enable-new-dtags -lSDL2 -lSDL2_mixer
GLIBS += -L$(GLDIR)/ncurses/lib $(GLDIR)/ncurses/lib/libncurses.a $(GLDIR)/ncurses/lib/libncursesw.a $(GLDIR)/ncurses/lib/libtinfo.a
GLIBS += -I$(GLDIR)/ncurses/include -Wl,-rpath,$(GLDIR)/ncurses/lib -Wl,--enable-new-dtags -lncurses -lncursesw -ltinfo



LDLIBS=  -lm
LDLIBS_PTHREAD= -lpthread
INCLUDE= ./Includes .
SOURCES= . ./Sources

SERVERINCLUDE= ./server/Includes ./extra_funcs/Includes
SERVERSOURCES= ./server/Sources ./server ./extra_funcs/Sources
SERVERRESOURCES= ./server/resources

CLIENTINCLUDE= ./client/Includes ./extra_funcs/Includes
CLIENTSOURCES= ./client/Sources ./client ./extra_funcs/Sources
CLIENTRESOURCES= ./client/resources

CLIENT_NOSDL_INCLUDE= ./client_no_sdl/Includes ./extra_funcs/Includes
CLIENT_NOSDL_SOURCES= ./client_no_sdl/Sources ./client_no_sdl ./extra_funcs/Sources
CLIENT_NOSDL_RESOURCES= ./client_no_sdl/resources


RESDIR= ./resources
 
CURRDIR=echo `pwd`
 
CC= cc
DEPFLAGS= -MP -MD

CFLAGS= -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -g $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS) 

SDLFLAGS= `sdl2-config --libs`

SOURCEFILES=$(foreach D,$(SOURCES), $(wildcard $(D)/*.c))

CLIENTSOURCEFILES=$(foreach D,$(CLIENTSOURCES), $(wildcard $(D)/*.c))

CLIENT_NOSDL_SOURCEFILES=$(foreach D,$(CLIENT_NOSDL_SOURCES), $(wildcard $(D)/*.c))

SERVERSOURCEFILES=$(foreach D,$(SERVERSOURCES), $(wildcard $(D)/*.c))



RESOURCEFILES=$(wildcard $(RESDIR)/*.o)

SERVERRESOURCEFILES=$(wildcard $(SERVERRESOURCES)/*.o)

CLIENT_NOSDL_RESOURCEFILES=$(wildcard $(CLIENT_NOSDL_RESOURCES)/*.o)

CLIENTRESOURCEFILES=$(wildcard $(CLIENTRESOURCES)/*.o)


OBJECTS=$(patsubst %.c,%.o,$(SOURCEFILES))

CLIENTOBJECTS=$(patsubst %.c,%.o,$(CLIENTSOURCEFILES))

CLIENT_NOSDL_OBJECTS=$(patsubst %.c,%.o,$(CLIENT_NOSDL_SOURCEFILES))

SERVEROBJECTS=$(patsubst %.c,%.o,$(SERVERSOURCEFILES))


ALLMODULES= $(RESOURCEFILES) $(OBJECTS) $(SERVEROBJECTS)  $(CLIENTOBJECTS) $(SERVERRESOURCEFILES) $(CLIENTRESOURCEFILES) $(CLIENT_NOSDL_OBJECTS) $(CLIENT_NOSDL_RESOURCEFILES)

SERVERMODULES= $(SERVEROBJECTS)  $(SERVERRESOURCEFILES)

CLIENTMODULES= $(CLIENTOBJECTS) $(CLIENTRESOURCEFILES)

CLIENT_NOSDL_MODULES= $(CLIENTO_NOSDL_OBJECTS) $(CLIENT_NOSDL_RESOURCEFILES)

DEPFILES= $(patsubst %.c,%.d,$(SOURCEFILES)) $(SERVERDEPFILES) $(CLIENTDEPFILES)


SERVERDEPFILES= $(patsubst %.c,%.d,$(SERVERSOURCEFILES))

CLIENTDEPFILES= $(patsubst %.c,%.d,$(CLIENTSOURCEFILES))

CLIENT_NOSDL_DEPFILES= $(patsubst %.c,%.d,$(CLIENT_NOSDL_SOURCEFILES))


CLIENTSTUFF=  $(CLIENTOBJECTS)

CLIENT_NOSDL_STUFF=  $(CLIENT_NOSDL_OBJECTS)

SERVERSTUFF= $(SERVEROBJECTS)

client: $(CLIENTBIN)
	echo $(LDLIBS)
	echo $(CURRDIR)

client_no_sdl: $(CLIENTBIN_NOSDL)
	echo $(LDLIBS)
	echo $(CURRDIR)

server: $(SERVERBIN)
	echo $(LDLIBS)
	echo $(CURRDIR)


main: $(BINARY)
	echo $(LDLIBS)
	echo $(CURRDIR)


$(BINARY): $(OBJECTS)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(RESOURCEFILES)  $(LDLIBS)

$(SERVERBIN): $(SERVERSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(SERVERRESOURCEFILES)  $(LDLIBS)

$(CLIENTBIN_NOSDL): $(CLIENT_NOSDL_STUFF)
	$(CC) -g -v  $(CFLAGS) $(SDLFLAGS) -o  $@ $^ $(CLIENT_NOSDL_RESOURCEFILES)  $(LDLIBS)

$(CLIENTBIN): $(CLIENTSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(CLIENTRESOURCEFILES)  $(LDLIBS) $(GLIBS) $(LDLIBS_PTHREAD)





%.o:%.c
	$(CC) -g  $(CFLAGS) -c -o $@ $<  $(LDLIBS)
	echo $(CFLAGS)

cleanall:
	rm -rf $(BINARY) $(SERVERBIN) $(CLIENTBIN) $(CLIENTBIN_NOSDL) $(ALLMODULES) $(DEPFILES)


cleanserver:
	rm -rf $(SERVERBIN) $(SERVERMODULES) $(DEPFILES) $(SERVERDEPFILES)


cleanclient:
	rm -rf $(CLIENTBIN) $(CLIENTMODULES) $(DEPFILES) $(CLIENTDEPFILES)

cleanclient_nosdl:
	rm -rf $(CLIENTBIN_NOSDL) $(CLIENT_NOSDL_MODULES) $(DEPFILES) $(CLIENT_NOSDL_DEPFILES)



