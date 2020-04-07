// blank main file

#include "server.hpp"
#include <thread>
#include <chrono>


int main()
{
	std::map<unsigned int, Account> db;
	Server theServer(db);

	std::cout << "=============Playground=============" << std::endl;
	theServer.test();
	std::cout << "=============Playground=============" << std::endl;

	// check & process requests every five seconds
	//using namespace std::chrono_literals;
	//while (true) {
	//	theServer.checkRequests();
	//	theServer.processRequests();
	//	std::this_thread::sleep_for(1s);
	//}
}
