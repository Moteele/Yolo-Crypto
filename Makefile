# adds protobuf default installation path for pkg-config
LIBCXXFLAGS:=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --cflags protobuf`
LIBLDFLAGS:=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --libs protobuf`

CXXFLAGS:=-std=c++14 $(LIBCXXFLAGS)
LDFLAGS:=-ldl $(LIBLDFLAGS)

SOURCES_SERVER=server/server.cpp server/message.pb.cpp utils/functions.cpp utils/userAcc.pb.cc utils/mess.pb.cc
OBJECTS_SERVER=$(SOURCES_MAIN:.cpp=.o)
SOURCES_SERVER_TEST=server/test-server.cpp server/test-main.cpp $(SOURCES_SERVER)
SOURCES_SERVER_MAIN=server/main.cpp $(SOURCES_SERVER)
OBJECTS_SERVER_TEST=$(SOURCES_SERVER_TEST:.cpp=.o)
OBJECTS_SERVER_MAIN=$(SOURCES_SERVER_MAIN:.cpp=.o)

DEPS=server/server.hpp server/util.hpp server/message.pb.h client/client.hpp utils/functions.h utils/constants.h utils/userAcc.pb.h utils/mess.pb.h

SOURCES_CLIENT=client/client.cpp utils/functions.cpp utils/userAcc.pb.cc utils/mess.pb.cc
OBJECTS_CLIENT=$(SOURCES_MAIN:.cpp=.o)
SOURCES_CLIENT_MAIN=client/main.cpp $(SOURCES_CLIENT)
OBJECTS_CLIENT_MAIN=$(SOURCES_CLIENT_MAIN:.cpp=.o)

#all: test
all: server-build client-build

test: test-server
	./test-server

server-build: serverApp
#	./main

client-build: clientApp

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
