#include "server.hpp"

#include <thread>
#include <chrono>


int main(int argc, char* argv[])
{
	if (argc > 1) {
	    if (std::string(argv[1]) == "-i") {
			std::cout << "Initialized new user database" << std::endl;
			initServer();
	    }
	}
	std::map<unsigned int, Account> db;
	Server theServer(db);

	// check & process requests every five seconds
	using namespace std::chrono_literals;

	/**
	 * NETWORKING
	 * */
    theServer.runServer();
}
