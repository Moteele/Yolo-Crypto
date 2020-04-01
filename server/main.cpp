// blank main file

#include "server.hpp"
#include <thread>

int main()
{
	std::map<unsigned int, Account> db;
	Server theServer(db);

	// check & process requests every five seconds
	while (true) {
		theServer.checkRequests();
		theServer.processRequests();
		std::this_thread.sleep_for(5s);
	}
}
