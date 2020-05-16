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
	// while (true) {
	// 	//theServer.loadUsers();
	// 	theServer.checkRequests();
	// 	theServer.processRequests();
	// 	//theServer.writeUsers();
	// 	std::this_thread::sleep_for(1s);
	// }

	/**
	 * NETWORKING
	 * */
	int opt = 1;
    int master_socket , addrlen , new_socket,
          max_clients = 30 , activity, i , valread , socketDescriptor;
    int maxSocketDescriptor;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K

    std::vector<int> client_socket(30, 0);

    int serverPort = 8080;

    //set of socket descriptors
    fd_set readfds;


    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        std::cerr << "Master socket initialization failed" << std::endl;
        exit(1);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        std::cerr << "Allowing multiple connections failed" << std::endl;
        exit(1);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( serverPort );

    //bind the socket to localhost port 8080
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        std::cerr << "Binding localhost:" << serverPort << " failed" << std::endl;
        exit(1);
    }
    std::cout << "Server listening on port " << serverPort << std::endl;

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        std::cerr << "Listening on localhost:" << serverPort << " failed" << std::endl;
        exit(1);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    std::cout << "Server running..." << std::endl;

    while(true)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        maxSocketDescriptor = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            socketDescriptor = client_socket[i];

            //if valid socket descriptor then add to read list
            if(socketDescriptor > 0)
                FD_SET( socketDescriptor , &readfds);

            //highest file descriptor number, need it for the select function
            if(socketDescriptor > maxSocketDescriptor)
                maxSocketDescriptor = socketDescriptor;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( maxSocketDescriptor + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            std::cerr << "Select error" << std::endl;
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                std::cerr << "Accepting conncection failed" << std::endl;
                exit(1);
            }


            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    std::cout << "Socket added " << i << std::endl;
                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            socketDescriptor = client_socket[i];

            if (FD_ISSET( socketDescriptor , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( socketDescriptor , buffer, 2048)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(socketDescriptor , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    std::cout << "Host disconnected " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port);

                    //Close the socket and mark as 0 in list for reuse
                    close( socketDescriptor );
                    client_socket[i] = 0;
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
					std::stringstream req;
					for (int i = 0; i < valread; ++i) {
						req << buffer[i];
					}
					theServer.readRequest(req.str());
					theServer.processRequests(socketDescriptor);
                    //std::cout << buffer << std::endl;
                    //send(socketDescriptor , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }
}
