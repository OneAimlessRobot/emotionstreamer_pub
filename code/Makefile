BINARY= emotionserver.exe

SERVERBIN= ./server/server.exe

CLIENTBIN= ./client/client.exe

BROWSERBIN= ./server_browser/server_browser.exe

HEARTBEATBIN= ./heartbeat/heart_beat.exe

MASTERBIN= ./master_server/master_server.exe

GLDIR= ./player

LDLIBS :=  -lm

LDLIBS_SOUND := -lasound -lpulse -lpulse-simple

LDLIBS_PTHREAD= -lpthread

LDLIBS_GRAPHICAL= -lncurses

LDLIBS_SQLITE= -lsqlite3

LDLIBS_CLIENT += $(LDLIBS_PTHREAD) $(LDLIBS_SOUND) $(LDLIBS_GRAPHICAL)

LDLIBS_SERVER += $(LDLIBS_PTHREAD)


INCLUDE= ./Includes .
SOURCES= . ./Sources

SERVERINCLUDE= ./server/Includes ./extra_funcs/Includes
SERVERSOURCES= ./server/Sources ./server ./extra_funcs/Sources
SERVERRESOURCES= ./server/resources

CLIENTINCLUDE= ./client/Includes ./extra_funcs/Includes
CLIENTSOURCES= ./client/Sources ./client ./extra_funcs/Sources
CLIENTRESOURCES= ./client/resources

BROWSERINCLUDE= ./server_browser/Includes ./extra_funcs/Includes
BROWSERSOURCES= ./server_browser/Sources ./server_browser ./extra_funcs/Sources
BROWSERRESOURCES= ./server_browser/resources

HEARTBEATINCLUDE= ./heartbeat/Includes ./extra_funcs/Includes
HEARTBEATSOURCES= ./heartbeat/Sources ./heartbeat ./extra_funcs/Sources
HEARTBEATRESOURCES= ./heartbeat/resources

MASTERINCLUDE= ./master_server/Includes ./extra_funcs/Includes
MASTERSOURCES= ./master_server/Sources ./master_server ./extra_funcs/Sources
MASTERRESOURCES= ./master_server/resources


RESDIR= ./resources

CURRDIR=echo `pwd`

CC= cc
#CC=clang
DEPFLAGS= -MP -MD

#CFLAGS += -fsanitize=thread -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -g $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS)
#CFLAGS += -fsanitize=address -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -g $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS)

CFLAGS += -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -g $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS)

CFLAGS2 :=  $(CFLAGS) $(SDL_CFLAGS) -O3


SOURCEFILES=$(foreach D,$(SOURCES), $(wildcard $(D)/*.c))

CLIENTSOURCEFILES=$(foreach D,$(CLIENTSOURCES), $(wildcard $(D)/*.c))

SERVERSOURCEFILES=$(foreach D,$(SERVERSOURCES), $(wildcard $(D)/*.c))

HEARTBEATSOURCEFILES=$(foreach D,$(HEARTBEATSOURCES), $(wildcard $(D)/*.c))

BROWSERSOURCEFILES=$(foreach D,$(BROWSERSOURCES), $(wildcard $(D)/*.c))

MASTERSOURCEFILES=$(foreach D,$(MASTERSOURCES), $(wildcard $(D)/*.c))



RESOURCEFILES=$(wildcard $(RESDIR)/*.o)

SERVERRESOURCEFILES=$(wildcard $(SERVERRESOURCES)/*.o)

CLIENTRESOURCEFILES=$(wildcard $(CLIENTRESOURCES)/*.o)

BROWSERRESOURCEFILES=$(wildcard $(BROWSERRESOURCES)/*.o)

HEARTBEATRESOURCEFILES=$(wildcard $(HEARTBEATRESOURCES)/*.o)

MASTERRESOURCEFILES=$(wildcard $(MASTERRESOURCES)/*.o)


OBJECTS=$(patsubst %.c,%.o,$(SOURCEFILES))

CLIENTOBJECTS=$(patsubst %.c,%.o,$(CLIENTSOURCEFILES))

SERVEROBJECTS=$(patsubst %.c,%.o,$(SERVERSOURCEFILES))

BROWSEROBJECTS=$(patsubst %.c,%.o,$(BROWSERSOURCEFILES))

HEARTBEATOBJECTS=$(patsubst %.c,%.o,$(HEARTBEATSOURCEFILES))

MASTEROBJECTS=$(patsubst %.c,%.o,$(MASTERSOURCEFILES))


ALLMODULES= $(RESOURCEFILES) $(OBJECTS) $(SERVEROBJECTS)  $(CLIENTOBJECTS) $(SERVERRESOURCEFILES) $(CLIENTRESOURCEFILES) $(HEARTBEATRESOURCES) $(HEARTBEATOBJECTS) $(BROWSERRESOURCES) $(BROWSEROBJECTS) $(MASTERRESOURCES) $(MASTEROBJECTS)

SERVERMODULES= $(SERVEROBJECTS)  $(SERVERRESOURCEFILES)

CLIENTMODULES= $(CLIENTOBJECTS) $(CLIENTRESOURCEFILES)

HEARTBEATMODULES= $(HEARTBEATOBJECTS) $(HEARTBEATRESOURCEFILES)

BROWSERMODULES= $(BROWSEROBJECTS) $(BROWSERRESOURCEFILES)

MASTERMODULES= $(MASTEROBJECTS) $(MASTERRESOURCEFILES)


DEPFILES= $(patsubst %.c,%.d,$(SOURCEFILES)) $(SERVERDEPFILES) $(CLIENTDEPFILES) $(MASTERDEPFILES)


SERVERDEPFILES= $(patsubst %.c,%.d,$(SERVERSOURCEFILES))

CLIENTDEPFILES= $(patsubst %.c,%.d,$(CLIENTSOURCEFILES))

HEARTBEATDEPFILES= $(patsubst %.c,%.d,$(HEARTBEATSOURCEFILES))

BROWSERDEPFILES= $(patsubst %.c,%.d,$(BROWSERSOURCEFILES))

MASTERDEPFILES= $(patsubst %.c,%.d,$(MASTERSOURCEFILES))



CLIENTSTUFF=  $(CLIENTOBJECTS)

SERVERSTUFF= $(SERVEROBJECTS)

BROWSERSTUFF= $(BROWSEROBJECTS)

HEARTBEATSTUFF= $(HEARTBEATOBJECTS)

MASTERSTUFF= $(MASTEROBJECTS)

client: $(CLIENTBIN)

server: $(SERVERBIN)

browser: $(HEARTBEATBIN)

heartbeat: $(BROWSERBIN)

master: $(MASTERBIN)


main: $(BINARY)


$(BINARY): $(OBJECTS)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(RESOURCEFILES) $(LDLIBS_SQLITE)

$(SERVERBIN): $(SERVERSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(SERVERRESOURCEFILES) $(LDLIBS_SERVER) $(LDLIBS_SQLITE)

$(CLIENTBIN): $(CLIENTSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(CLIENTRESOURCEFILES) $(LDLIBS_CLIENT) $(LDLIBS) $(LDLIBS_SQLITE)

$(BROWSERBIN): $(BROWSERSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(BROWSERRESOURCEFILES) $(LDLIBS_SERVER) $(LDLIBS_SQLITE)

$(HEARTBEATBIN): $(HEARTBEATSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(HEARTBEATRESOURCEFILES) $(LDLIBS_SERVER) $(LDLIBS_SQLITE)

$(MASTERBIN): $(MASTERSTUFF)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(MASTERRESOURCEFILES) $(LDLIBS_SERVER) $(LDLIBS_SQLITE)





%.o:%.c
	$(CC) -g  $(CFLAGS) -c -o $@ $<
	echo $(CFLAGS)

cleanall:
	rm -rf $(BINARY) $(SERVERBIN) $(CLIENTBIN)  $(ALLMODULES) $(DEPFILES)


cleanserver:
	rm -rf $(SERVERBIN) $(SERVERMODULES) $(DEPFILES) $(SERVERDEPFILES)


cleanclient:
	rm -rf $(CLIENTBIN) $(CLIENTMODULES) $(DEPFILES) $(CLIENTDEPFILES)


cleanbrowser:
	rm -rf $(BROWSERBIN) $(BROWSERMODULES) $(DEPFILES) $(BROWSERDEPFILES)

cleanheartbeat:
	rm -rf$(HEARTBEATBIN) $(HEARTBEATMODULES) $(DEPFILES) $(HEARTBEATDEPFILES)

cleanmaster:
	rm -rf$(MASTERBIN) $(MASTERMODULES) $(DEPFILES) $(MASTERDEPFILES)



