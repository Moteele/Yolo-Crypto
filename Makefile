SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c

# adds protobuf default installation path for pkg-config
LIBCXXFLAGS:=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --cflags protobuf`
LIBLDFLAGS:=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --libs protobuf`

CXXFLAGS:=-std=c++14 -Ilibs/include -Ilibs/openssl_internals -Llibs -pthread $(LIBCXXFLAGS)
LDFLAGS:=$(LIBLDFLAGS) -lcrypto -ldl

# generate dependency files for all *.cpp files
SOURCES_ALL=$(wildcard server/*.cpp) $(wildcard client/*.cpp) $(wildcard utils/*.cpp)
DEPS=$(SOURCES_ALL:.cpp=.d)

SOURCES_UTIL_TEST=utils/test-util.cpp server/test-main.cpp utils/util.cpp utils/key.cpp
OBJECTS_UTIL_TEST=$(SOURCES_UTIL_TEST:.cpp=.o)
SOURCES_SERVER=utils/mess.pb.cpp server/message.pb.cpp server/server.cpp utils/functions.cpp utils/userAcc.pb.cpp utils/util.cpp utils/key.cpp
SOURCES_SERVER_TEST=server/test-server.cpp server/test-main.cpp $(SOURCES_SERVER)
SOURCES_SERVER_MAIN=server/main.cpp $(SOURCES_SERVER)
OBJECTS_SERVER_TEST=$(SOURCES_SERVER_TEST:.cpp=.o)
OBJECTS_SERVER_MAIN=$(SOURCES_SERVER_MAIN:.cpp=.o)

SOURCES_CLIENT=client/client.cpp utils/functions.cpp utils/userAcc.pb.cpp utils/mess.pb.cpp
SOURCES_CLIENT_MAIN=client/main.cpp $(SOURCES_CLIENT)
OBJECTS_CLIENT_MAIN=$(SOURCES_CLIENT_MAIN:.cpp=.o)

RUN=`./utils/generate_protobuf.sh`


#all: test
all:  server-build client-build

test: all test-server test-util
	./test-server; ./test-util

test-valgrind: all test-server test-util
	valgrind --leak-check=full --show-reachable=yes ./test-server; valgrind --leak-check=full --show-reachable=yes ./test-util

	#valgrind --leak-check=full ./test-server


debug: CXXFLAGS += -DDEBUG -Og -ggdb
debug: server-build client-build test-util

server-build: serverApp
#	./main
client-build: clientApp

valgrind: server-build
	valgrind --leak-check=full --show-reachable=yes ./serverApp

debug-flags:
	echo $(CXXFLAGS); echo $(LDFLAGS); echo $(SOURCES_ALL)

test-util: $(OBJECTS_UTIL_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test-server: $(OBJECTS_SERVER_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

serverApp: $(OBJECTS_SERVER_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clientApp: $(OBJECTS_CLIENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: *.pb.cpp %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $< $(LDFLAGS)

%.pb.cpp: %.proto
	./utils/generate_protobuf.sh $^

clean:
	rm -rf $(OBJECTS_SERVER_TEST) $(OBJECTS_CLIENT) $(OBJECTS_CLIENT_MAIN) $(OBJECTS_UTIL_TEST) $(DEPS)

-include $(DEPS)
