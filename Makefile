# adds protobuf default installation path for pkg-config
LIBCXXFLAGS:=#`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --cflags protobuf`
LIBLDFLAGS:=#`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --libs protobuf`

CXXFLAGS:=-std=c++14 -Ilibs/include -Llibs $(LIBCXXFLAGS)
LDFLAGS:=-lmbedcrypto $(LIBLDFLAGS)

SOURCES_SERVER=server/server.cpp #server/message.pb.cpp
OBJECTS_SERVER=$(SOURCES_MAIN:.cpp=.o)
SOURCES_SERVER_TEST=server/test-server.cpp server/test-main.cpp $(SOURCES_SERVER)
SOURCES_SERVER_MAIN=server/main.cpp $(SOURCES_SERVER)
OBJECTS_SERVER_TEST=$(SOURCES_SERVER_TEST:.cpp=.o)
OBJECTS_SERVER_MAIN=$(SOURCES_SERVER_MAIN:.cpp=.o)
DEPS=server/server.hpp server/util.hpp client/client.hpp #server/message.pb.h

SOURCES_CLIENT=client/client.cpp
OBJECTS_CLIENT=$(SOURCES_MAIN:.cpp=.o)
SOURCES_CLIENT_MAIN=client/main.cpp $(SOURCES_CLIENT)
OBJECTS_CLIENT_MAIN=$(SOURCES_CLIENT_MAIN:.cpp=.o)

#all: test
all: server-build client-build

test: test-server
	valgrind --leak-check=full --show-reachable=yes ./test-server

server-build: serverApp
#	./main

client-build: clientApp

valgrind:server-build
	valgrind --leak-check=full --show-reachable=yes ./serverApp

debug:
	echo $(CXXFLAGS); echo $(LDFLAGS)

test-server: $(OBJECTS_SERVER_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

serverApp: $(OBJECTS_SERVER_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clientApp: $(OBJECTS_CLIENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(OBJECTS_SERVER_TEST)
