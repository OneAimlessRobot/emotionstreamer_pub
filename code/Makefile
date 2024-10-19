BINARY= emotionserver.exe

SERVERBIN= ./server/server.exe

CLIENTBIN= ./client/client.exe

GLDIR= ./player

GLIBS := -L$(GLDIR)/SDL2/lib $(GLDIR)/SDL2/lib/libSDL2.a  $(GLDIR)/SDL2/lib/libSDL2_mixer.a -lm -ldl -lpthread
GLIBS += -I$(GLDIR)/SDL2/include -Wl,-rpath,$(GLDIR)/SDL2/lib -Wl,--enable-new-dtags -lSDL2 -lSDL2_mixer
GLIBS += -L$(GLDIR)/ncurses/lib $(GLDIR)/ncurses/lib/libncurses.a $(GLDIR)/ncurses/lib/libncursesw.a $(GLDIR)/ncurses/lib/libtinfo.a
GLIBS += -I$(GLDIR)/ncurses/include -Wl,-rpath,$(GLDIR)/ncurses/lib -Wl,--enable-new-dtags -lncurses -lncursesw -ltinfo

SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

LDFLAGS := $(SDL_LDFLAGS) -lSDL2_mixer

SDLFLAGS := $(pkg-config sdl2 --cflags) `-Wl, --enable-new-dtags`
SDLLIBS :=  $(pkg-config sdl2 --libs) -lSDL2_mixer

LDLIBS :=  -lm


LDLIBS_PTHREAD= -lpthread

LDLIBS2 += $(LDLIBS) $(LDLIBS_PTHREAD) $(LDFLAGS)

INCLUDE= ./Includes .
SOURCES= . ./Sources

SERVERINCLUDE= ./server/Includes ./extra_funcs/Includes
SERVERSOURCES= ./server/Sources ./server ./extra_funcs/Sources
SERVERRESOURCES= ./server/resources

CLIENTINCLUDE= ./client/Includes ./extra_funcs/Includes
CLIENTSOURCES= ./client/Sources ./client ./extra_funcs/Sources
CLIENTRESOURCES= ./client/resources


RESDIR= ./resources

CURRDIR=echo `pwd`

CC= cc
DEPFLAGS= -MP -MD

CFLAGS += -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -g $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS)

CFLAGS2 :=  $(CFLAGS) $(SDL_CFLAGS) -O3


SOURCEFILES=$(foreach D,$(SOURCES), $(wildcard $(D)/*.c))

CLIENTSOURCEFILES=$(foreach D,$(CLIENTSOURCES), $(wildcard $(D)/*.c))

SERVERSOURCEFILES=$(foreach D,$(SERVERSOURCES), $(wildcard $(D)/*.c))



RESOURCEFILES=$(wildcard $(RESDIR)/*.o)

SERVERRESOURCEFILES=$(wildcard $(SERVERRESOURCES)/*.o)

CLIENTRESOURCEFILES=$(wildcard $(CLIENTRESOURCES)/*.o)


OBJECTS=$(patsubst %.c,%.o,$(SOURCEFILES))

CLIENTOBJECTS=$(patsubst %.c,%.o,$(CLIENTSOURCEFILES))

SERVEROBJECTS=$(patsubst %.c,%.o,$(SERVERSOURCEFILES))


ALLMODULES= $(RESOURCEFILES) $(OBJECTS) $(SERVEROBJECTS)  $(CLIENTOBJECTS) $(SERVERRESOURCEFILES) $(CLIENTRESOURCEFILES)

SERVERMODULES= $(SERVEROBJECTS)  $(SERVERRESOURCEFILES)

CLIENTMODULES= $(CLIENTOBJECTS) $(CLIENTRESOURCEFILES)

DEPFILES= $(patsubst %.c,%.d,$(SOURCEFILES)) $(SERVERDEPFILES) $(CLIENTDEPFILES)


SERVERDEPFILES= $(patsubst %.c,%.d,$(SERVERSOURCEFILES))

CLIENTDEPFILES= $(patsubst %.c,%.d,$(CLIENTSOURCEFILES))



CLIENTSTUFF=  $(CLIENTOBJECTS)

SERVERSTUFF= $(SERVEROBJECTS)

client: $(CLIENTBIN)
	echo $(SDL_CFLAGS)
	echo $(SDL_LDFLAGS)
server: $(SERVERBIN)
	echo $(LDLIBS)
	echo $(CURRDIR)


main: $(BINARY)
	echo $(LDLIBS)
	echo $(CURRDIR)


$(BINARY): $(OBJECTS)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(RESOURCEFILES)  $(LDLIBS2)

$(SERVERBIN): $(SERVERSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(SERVERRESOURCEFILES) $(LDLIBS2) 

$(CLIENTBIN): $(CLIENTSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(CLIENTRESOURCEFILES)    $(LDLIBS2)





%.o:%.c
	$(CC) -g  $(CFLAGS) -c -o $@ $<  $(LDLIBS)
	echo $(CFLAGS)

cleanall:
	rm -rf $(BINARY) $(SERVERBIN) $(CLIENTBIN)  $(ALLMODULES) $(DEPFILES)


cleanserver:
	rm -rf $(SERVERBIN) $(SERVERMODULES) $(DEPFILES) $(SERVERDEPFILES)


cleanclient:
	rm -rf $(CLIENTBIN) $(CLIENTMODULES) $(DEPFILES) $(CLIENTDEPFILES)




