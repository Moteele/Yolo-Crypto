# adds protobuf default installation path for pkg-config
LIBCXXFLAGS:=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --cflags protobuf`
LIBLDFLAGS:=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --libs protobuf`

CXXFLAGS:=-std=c++14 -Ilibs/include -Ilibs/openssl_internals -Llibs -pthread $(LIBCXXFLAGS)
LDFLAGS:=$(LIBLDFLAGS) -lcrypto -ldl -no-pie

SOURCES_UTIL_TEST=utils/test-util.cpp server/test-main.cpp
OBJECTS_UTIL_TEST=$(SOURCES_UTIL_TEST:.cpp=.o)
SOURCES_SERVER=server/server.cpp utils/message.pb.cpp
OBJECTS_SERVER=$(SOURCES_MAIN:.cpp=.o)
SOURCES_SERVER_TEST=server/test-server.cpp server/test-main.cpp $(SOURCES_SERVER)
SOURCES_SERVER_MAIN=server/main.cpp $(SOURCES_SERVER)
OBJECTS_SERVER_TEST=$(SOURCES_SERVER_TEST:.cpp=.o)
OBJECTS_SERVER_MAIN=$(SOURCES_SERVER_MAIN:.cpp=.o)
OPENSSL_EXTRACTED=libs/openssl_internals/curve25519.h
DEPS=$(OPENSSL_EXTRACTED) server/server.hpp utils/util.hpp client/client.hpp utils/message.pb.h

SOURCES_CLIENT=client/client.cpp utils/message.pb.cpp
OBJECTS_CLIENT=$(SOURCES_MAIN:.cpp=.o)
SOURCES_CLIENT_MAIN=client/main.cpp $(SOURCES_CLIENT)
OBJECTS_CLIENT_MAIN=$(SOURCES_CLIENT_MAIN:.cpp=.o)

#all: test
all: server-build client-build

test: test-server test-util
	./test-server; ./test-util

test-valgrind: test-server
	valgrind --leak-check=full --show-reachable=yes ./test-server

	#valgrind --leak-check=full ./test-server


server-build: serverApp
#	./main

client-build: clientApp

valgrind:server-build
	valgrind --leak-check=full --show-reachable=yes ./serverApp

debug:
	echo $(CXXFLAGS); echo $(LDFLAGS)

test-util: $(OBJECTS_UTIL_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test-server: $(OBJECTS_SERVER_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

serverApp: $(OBJECTS_SERVER_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clientApp: $(OBJECTS_CLIENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(OBJECTS_SERVER_TEST) $(OBJECTS_CLIENT) $(OBJECTS_CLIENT_MAIN)
