SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c

# adds protobuf default installation path for pkg-config
LIBCXXFLAGS=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --cflags protobuf`
LIBLDFLAGS=`export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig; pkg-config --libs protobuf`

CXXFLAGS=-std=c++14 -Ilibs/include -Ilibs/openssl_internals -Llibs -pthread $(LIBCXXFLAGS) $(OPTIONAL)
LDFLAGS=$(LIBLDFLAGS) -lcrypto -ldl

# generate dependency files for all *.cpp files
SOURCES_ALL=$(wildcard server/*.cpp) $(wildcard client/*.cpp) $(wildcard utils/*.cpp)
DEPS=$(SOURCES_ALL:.cpp=.d)

# UTIL
SOURCES_UTIL_TEST=utils/test-util.cpp server/test-main.cpp utils/util.cpp utils/key.cpp
OBJECTS_UTIL_TEST=$(SOURCES_UTIL_TEST:.cpp=.o)

# SERVER
SOURCES_SERVER=utils/mess.pb.cpp server/message.pb.cpp server/server.cpp utils/functions.cpp utils/userAcc.pb.cpp utils/util.cpp utils/key.cpp
SOURCES_SERVER_TEST=server/test-server.cpp server/test-main.cpp $(SOURCES_SERVER)
SOURCES_SERVER_MAIN=server/main.cpp $(SOURCES_SERVER)

OBJECTS_SERVER_TEST=$(SOURCES_SERVER_TEST:.cpp=.o)
OBJECTS_SERVER_MAIN=$(SOURCES_SERVER_MAIN:.cpp=.o)

# CLIENT
SOURCES_CLIENT=client/client.cpp utils/functions.cpp utils/userAcc.pb.cpp utils/mess.pb.cpp utils/key.cpp utils/util.cpp
SOURCES_CLIENT_TEST=client/test-client.cpp server/test-main.cpp $(SOURCES_CLIENT)
SOURCES_CLIENT_MAIN=client/main.cpp $(SOURCES_CLIENT)

OBJECTS_CLIENT=$(SOURCES_MAIN:.cpp=.o)
OBJECTS_CLIENT_TEST=$(SOURCES_CLIENT_TEST:.cpp=.o)
OBJECTS_CLIENT_MAIN=$(SOURCES_CLIENT_MAIN:.cpp=.o)

# FUNCTIONS
SOURCES_FUNCTIONS_TEST=utils/test-functions.cpp server/test-main.cpp utils/functions.cpp
OBJECTS_FUNCTIONS_TEST=$(SOURCES_FUNCTIONS_TEST:.cpp=.o)

# PROTO FILES
PROTO_FILES=$(wildcard utils/*.proto)
PROTO_FILES_CPP=$(PROTO_FILES:.proto=.pb.cpp)

############
# RECIPIES #
############
all:  server-build client-build

test-base: all test-server test-util test-functions test-client

test: test-base
	./test-server
	./test-util
	./test-functions
	./test-client

test-valgrind: test-base
	valgrind --leak-check=full --show-reachable=yes ./test-server
	valgrind --leak-check=full --show-reachable=yes ./test-util
	valgrind --leak-check=full --show-reachable=yes ./test-functions
	valgrind --leak-check=full --show-reachable=yes ./test-client


# builds a target with debugging flags
# example: make debug-test ... builds target test with debugging flags -DDEBUG -Og -ggdb
debug-%: clean
	$(MAKE) $* $(MAKEFILE) OPTIONAL="$(OPTIONAL) -DDEBUG -Og -ggdb"

# builds a target with coverage flags
# WARNING: for generating html files, 'lcov' is required
coverage-%: clean
	$(MAKE) $* $(MAKEFILE) CXX="g++" OPTIONAL="$(OPTIONAL) --coverage"
	./utils/generate_lcov.sh

# builds a target with profiling flags
# generates file 'gmon.out', which can be processed with 'gprof'
profile-%: clean
	$(MAKE) $* $(MAKEFILE) OPTIONAL="$(OPTIONAL) -pg"

# prints said variable
print-%:
	@echo '$*=$($*)'

server-build: serverApp

client-build: clientApp

valgrind: server-build
	valgrind --leak-check=full --show-reachable=yes ./serverApp

flags:
	@echo $(CXXFLAGS)
	@echo $(LDFLAGS)

test-util: $(OBJECTS_UTIL_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test-server: $(OBJECTS_SERVER_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test-client: $(OBJECTS_CLIENT_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test-functions: $(OBJECTS_FUNCTIONS_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

serverApp: $(OBJECTS_SERVER_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clientApp: $(OBJECTS_CLIENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(PROTO_FILES_CPP)
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $< $(LDFLAGS)

%.pb.cpp: %.proto
	./utils/generate_protobuf.sh $<

# CLEANUP
.PHONY: clean clean-coverage

clean: clean-coverage
	@rm -rf $(OBJECTS_SERVER_TEST) $(OBJECTS_CLIENT) $(OBJECTS_CLIENT_MAIN) $(OBJECTS_UTIL_TEST) $(DEPS)

clean-coverage:
	@rm -rf `find './' -name "*.gcov"`
	@rm -rf `find './' -name "*.gcda"`
	@rm -rf `find './' -name "*.gcno"`

-include $(DEPS)
