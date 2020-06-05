#include "server.hpp"
#include "../utils/key.hpp"


void Account::print(std::ostream &os) const
{
	os << "id: " << id_ << std::endl;
	os << "login: " << login_ << std::endl;
	os << "name: " << displayName_ << std::endl;

}

unsigned int Server::createAccount(const std::string &login, const std::string &pwdHash, const std::string &name)
{
	for (auto &acc : database_) {
		if (acc.second.getLogin() == login) {
			return 0;
		}
	}

	database_.emplace(id_, Account(id_, login, pwdHash, name));
	++id_;

	return id_ - 1;
}

bool Server::deleteAccount(unsigned int id)
{
	auto it = database_.find(id);

	if (it == database_.end()) {
		return false;
	}

	database_.erase(it);

	return true;
}

bool Server::signIn(const std::string &login, const std::string &pwdHash, const unsigned int challenge)
{
	unsigned int id = 0;
	std::string dbHash;

	for (auto &acc : database_) {
		if (acc.second.getLogin() == login) {
			id = acc.first;
			dbHash = acc.second.getPwdHash();
			break;
		}
	}

	dbHash = dbHash + std::to_string(challenge);

	if (id == 0 || online_.find(id) != online_.end()) {
		return false;
	}

	std::string hash;

	Util::hash512(dbHash, dbHash.size(), hash);

	if (hash != pwdHash) {
		return false;
	}

	online_.insert(id);
	return true;
}

bool Server::signOut(unsigned int id)
{
	auto it = online_.find(id);

	if (it == online_.end()) {
		return false;
	}

	online_.erase(it);

	return true;
}

void Server::listUsers(std::ostream &os) const
{
	os << "Registered users:" << std::endl;

	for (auto &acc : database_) {
		acc.second.print(os);
	}

	os << std::endl;
}

void Server::listOnline(std::ostream &os) const
{
	os << "Online users" << std::endl;

	for (auto id: online_) {
		database_.find(id)->second.print(os);
	}

	os << std::endl;
}

void Server::test()
{

}

void Server::readRequest(const std::string &req) {
	requests_.emplace_back(req);
	//std::cout << "got request: " << req << std::endl;
}

int Server::tryCreateAccount(int socketDescriptor, const std::string &request) {
	int delim = 0;
	char del = ';';
	std::string tmp = request;
	delim = request.find_first_of(del);
	std::string name = request.substr(0, delim);
	return 0;
}

void Server::processRequests(int socketDescriptor)
{
	Mess message;
	for (int i = 0; i < requests_.size(); ++i) {
		//std::cout << "processing request" << requests_[i] << std::endl;
		if (requests_[i] == "") {
			continue;
		}

		message.ParseFromString(hexToString(requests_[i]));
		switch (message.type()) {
		case Mess::MESSAGE:
			performSendMessage(socketDescriptor, requests_[i]);
			break;
		case Mess::AUTH:
			performAuth(socketDescriptor, requests_[i]);
			break;
		case Mess::FETCH_MESSAGES:
			performFetchMessages(socketDescriptor, requests_[i]);
			break;
		case Mess::FETCH_KEYS:
			performFetchKeys(socketDescriptor, requests_[i]);
			break;
		case Mess::INIT_MESSAGE:
			performSendInitialMsg(socketDescriptor, requests_[i]);
			break;
		case Mess::CREATE_ACCOUNT:
			performCreateAccount(socketDescriptor, requests_[i]);
			break;
		default:
			break;
		}
	}

	requests_.clear();
}

void Server::performSendMessage(int socketDescriptor, const std::string &req) {
	std::string serialized;
	Mess message;
	Mess response;
	message.ParseFromString(hexToString(req));
	response.set_type(Mess::MESSAGE);
	response.set_reciever(message.sender());


	// find sender
	int sendIndex = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == message.sender()) {
			sendIndex = i;
			break;
		}
	}

	// sender not found :D
	if (sendIndex == -1) {
		throw std::runtime_error("what the fuck did just happen?!");
	}

	// find reciever
	int recIndex = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == message.reciever()) {
			recIndex = i;
			break;
		}
	}
	// reciever not found
	if (recIndex == -1) {
		response.set_error(true);
		response.SerializeToString(&serialized);
		send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);
		return;
	}
	message.set_error(false);

	message.SerializeToString(&serialized);

	// add messages to corresponding users
	users_[recIndex].add_messages(stringToHex(serialized));
	users_[sendIndex].add_messages(stringToHex(serialized));

	response.SerializeToString(&serialized);
	send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);
}

void Server::performAuth(int socketDescriptor, const std::string &req) {
	//std::cout << "performing auth ... ";

	Mess message;
	Mess response;
	std::string serialized;
	message.ParseFromString(hexToString(req));
	response.set_type(Mess::AUTH);
	response.set_reciever(message.username());
	response.set_error(true);


	// find user
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == message.username()) {
			if (users_[i].pwdhash() == message.password()) {
				response.set_error(false);
				//std::cout << "DEBUG: auth success";
			}
			break;
		}
	}


	// write response for sender
	response.SerializeToString(&serialized);
	send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);


	//std::cout << "OK" << std::endl;
}

void Server::performFetchMessages(int socketDescriptor, const std::string &req) {
	//std::cout << "DEBUG: fetching messages" << std::endl;
	std::string serialized;
	Mess message;
	message.ParseFromString(hexToString(req));

	int index = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == message.sender()) {
			index = i;
			break;
		}
	}

	Mess fetched;
	auto messages = users_[index].messages();
	// parse message, change the type and write it to responses
	for (const auto &it : messages) {
		fetched.ParseFromString(hexToString(it));
		fetched.set_type(Mess::FETCH_MESSAGES);
		fetched.SerializeToString(&serialized);
		send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);
		fetched.clear_textcontent();
	}

	//std::cout << "DEBUG: messages fetched" << std::endl;
}

void Server::performSendInitialMsg(int socketDescriptor, const std::string &req) {
	send(socketDescriptor, req.c_str(), req.size(), 0);
}

void Server::performFetchKeys(int socketDescriptor, const std::string &req) {
	//std::cout << "fetching keys..." << std::endl;
	std::string serialized;
	Mess response;
	Mess message;
	message.ParseFromString(hexToString(req));
	response.set_type(Mess::FETCH_KEYS);
	response.set_reciever(message.sender());
	response.set_username(message.username());


	// find user
	int index = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == message.username()) {
			index = i;
			break;
		}
	}
	// user not found, can't fetch key bundle
	if (index == -1) {
		response.set_error(true);
		response.SerializeToString(&serialized);
		send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);
		//std::cout << "failed!" << std::endl;
		return;
	}

	// fill out necessery values
	response.set_reqidkey(users_[index].publicik());
	response.set_reqprekey(users_[index].publicpk());
	response.set_reqprekeysig(users_[index].signedpk());

	////TODO: take only one and delete it from server
	response.add_senotp(users_[index].onetimepublic()[0]);

	response.SerializeToString(&serialized);
	send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);
	//std::cout << "OK" << std::endl;
}

void Server::performCreateAccount(int socketDescriptor, const std::string &req) {
	//std::cout << "creating account ... " << std::endl;
	Mess message;
	Mess response;
	message.ParseFromString(hexToString(req));
	std::string serialized;

	response.set_type(Mess::CREATE_ACCOUNT);
	response.set_reciever(message.username());
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == message.username()) {
			// duplicit name
			response.set_error(true);
			response.SerializeToString(&serialized);
			send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);
			//std::cout << "failed!" << std::endl;
			return;
		}
	}

	// save user
	userAcc newUser;
	newUser.set_name(message.username());
	newUser.set_id(users_.size());
	newUser.set_pwdhash(message.password());
	newUser.set_publicik(message.senidkey());
	newUser.set_publicpk(message.senprekey());
	newUser.set_signedpk(message.senprekeysig());
	for (auto &el : message.senotp()) {
		newUser.add_onetimepublic(el);
	}

	users_.push_back(newUser);

	response.set_error(false);
	response.set_senid(newUser.id());


	// write response
	response.SerializeToString(&serialized);
	send(socketDescriptor, stringToHex(serialized).c_str(), stringToHex(serialized).size(), 0);
	//std::cout << "OK" << std::endl;
}

void Server::loadUsers() {
	const std::string userDbPath = "tmp_files/db/users";
	users_.clear();
	std::ifstream usersFile(userDbPath);
	std::string line;
	while (std::getline(usersFile, line)) {
		std::string parsedFromHex = hexToString(line);
		userAcc user;
		user.ParseFromString(parsedFromHex);
		std::cout << "load user with name:" << user.name() << std::endl;
		users_.push_back(user);
	}
	std::cout << "current number of users: " << users_.size() << std::endl;
	usersFile.close();
}

void Server::writeUsers() {
	const std::string usersDbPath = "tmp_files/db/users";
	std::ofstream usersFile(usersDbPath, std::ofstream::out | std::ofstream::trunc);
	std::string line;
	for (int i = 0; i < users_.size(); ++i) {

		if (users_[i].name().size() < 1) {
			continue;
		}

		std::cout << "writting user with name:" << users_[i].name() << std::endl;

		users_[i].SerializeToString(&line);

		usersFile << stringToHex(line);
		usersFile << '\n';
	}
	usersFile.close();
}

void Server::runServer() {
	for (int i = 0; i < 30; ++i) {
		sockets_.push_back(0);
	}

	int opt = 1;
    int master_socket , addrlen , new_socket,
          max_clients = 30 , activity, i , valread , socketDescriptor;
    int maxSocketDescriptor;
    struct sockaddr_in address;

    char buffer[2049];  //data buffer of 1K

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
			socketDescriptor = sockets_[i];

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
				if( sockets_[i] == 0 )
                {
                    sockets_[i] = new_socket;
                    std::cout << "Socket added " << i << std::endl;
                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
			socketDescriptor = sockets_[i];

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
					sockets_[i] = 0;
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
					readRequest(req.str());
					processRequests(socketDescriptor);
					writeUsers();
					loadUsers();
                }
            }
        }
    }
}
