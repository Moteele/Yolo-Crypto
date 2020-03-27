CXXFLAGS=-std=c++14 -pthread -Wl,--no-as-needed -lprotobuf -ldl
SOURCES_SERVER=server/server.cpp server/message.pb.cpp
OBJECTS_SERVER=$(SOURCES_MAIN:.cpp=.o)
SOURCES_SERVER_TEST=server/test-server.cpp server/test-main.cpp $(SOURCES_SERVER)
OBJECTS_SERVER_TEST=$(SOURCES_SERVER_TEST:.cpp=.o)
DEPS=server/server.hpp server/util.hpp server/message.pb.h

all: test

test: test-server
	./test-server

test-server: $(OBJECTS_SERVER_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJECTS_SERVER_TEST)
