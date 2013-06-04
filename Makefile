COM=CommonLibs
SQL=sqlite3
SR=.
LOCALLIBS=$(COM)/Logger.cpp $(COM)/Timeval.cpp $(COM)/Threads.cpp $(COM)/Sockets.cpp $(COM)/Configuration.cpp $(COM)/sqlite3util.cpp $(SR)/SubscriberRegistry.cpp $(COM)/Utils.cpp servershare.cpp
LIBS= -L$(SQL) $(LOCALLIBS) -losipparser2 -losip2 -lc -lpthread -lsqlite3
INCLUDES=-I$(COM) -I$(SQL) -I$(SR)
CPPFLAGS=-g -Wall -Wno-deprecated

all: sipauthserve

sipauthserve: sipauthserve.cpp $(LOCALLIBS)
	g++ -o sipauthserve $(CPPFLAGS) $(INCLUDES) sipauthserve.cpp $(LIBS)

clean:
	rm -f comp128 subscriberserver.cgi srmanager.cgi sipauthserve test.SubscriberRegistry/test
	rm -r -f *.dSYM
