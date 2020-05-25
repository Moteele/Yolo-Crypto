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

/*void Server::checkRequests()
{
	std::string path = "tmp_files/req";
	std::vector<std::string> requestFiles = getUnlockedFiles(path);

	// process createAcc requests
	for (int i = 0; i < requestFiles.size(); ++i) {
		if (requestFiles[i] == "req_master") {
			continue;
		}
		int accCreation = tryCreateAccount(requestFiles[i]);
		lockFile("tmp_files/res/" + requestFiles[i]);
		std::ofstream resFile("tmp_files/res/" + requestFiles[i], std::ofstream::out | std::ofstream::trunc);
		if (accCreation == -1) {
			resFile << "Error while creating account" << std::endl;
		} else {
			resFile << "Success" << std::endl;
		}
		resFile.close();
		unlockFile("tmp_files/res/" + requestFiles[i]);
	}

	while (isFileLocked(REQEST_FILE_PATH)) {
		// waiting
	}

	lockFile(REQEST_FILE_PATH);

	// read and store requests
	std::ifstream reqFile(REQEST_FILE_PATH);
	std::string line;
	while (std::getline(reqFile, line)) {
		requests_.emplace_back(line);
	}
	reqFile.close();

	// erase requests from request file
	std::ofstream eraseReqFile(REQEST_FILE_PATH, std::ofstream::out | std::ofstream::trunc);
	eraseReqFile.close();

	unlockFile(REQEST_FILE_PATH);
}*/

void Server::readRequest(const std::string &req) {
	requests_.emplace_back(req);
	std::cout << "got request: " << req << std::endl;
}

int Server::findSocketIndexFromSocket(int socketDescriptor) {
	for (int i = 0; i < sockets_.size(); ++i) {
		// if (sockets_[i].first == socketDescriptor) {
		// 	return i;
		// }
	}
	return -1;
}

int Server::findSocketIndexFromName(const std::string &name) {
	for (int i = 0; i < sockets_.size(); ++i) {
		// if (sockets_[i].second == name) {
		// 	return i;
		// }
	}
	return -1;
}

int Server::tryCreateAccount(int socketDescriptor, const std::string &request) {
	int delim = 0;
	char del = ';';
	std::string tmp = request;
	delim = request.find_first_of(del);
	std::string name = request.substr(0, delim);

	userAcc newUser;
	newUser.set_id(users_.size());
	newUser.set_name(name);
	for (int i = 0; i < 14; ++i) {
		delim = tmp.find_first_of(del);
		std::string currentLine = tmp.substr(0, delim);

		int split = -1;
		split = currentLine.find_first_of(':');
		std::cout << "currentLine: " << currentLine << std::endl;
		if (split == -1) {
			std::cout << "continue" << std::endl;
			tmp = tmp.substr(delim + 1);
			continue;
		}
		std::string command = currentLine.substr(0, split);
		std::string value = currentLine.substr(split + 1);
		std::cout << "command: " << command << " value: " << value << std::endl;
		if (command == "createAccount") {
			newUser.set_name(value);
		}
		if (command == "password") {
			newUser.set_pwdhash(value);
		}
		if (command == "privateId") {
			newUser.set_privateik(value);
		}
		if (command == "publicId") {
			newUser.set_publicik(value);
		}
		if (command == "privateSignedPrekey") {
			newUser.set_privatepk(value);
		}
		if (command == "publicSignedPrekey") {
			newUser.set_publicpk(value);
		}
		if (command == "prekeySignature") {
			newUser.set_signedpk(value);
		}
		if (command == "privateOnetime") {
			newUser.add_onetimeprivate(value);
		}
		if (command == "publicOnetime") {
			newUser.add_onetimepublic(value);
		}
		tmp = tmp.substr(delim + 1);
	}
	std::string result = name + ";auth;Success";
	users_.push_back(newUser);

	int socketIndex = findSocketIndexFromSocket(socketDescriptor);
	// sockets_[socketIndex].second = name;

	send(socketDescriptor, result.c_str(), result.size(), 0);
}

void Server::processRequests(int socketDescriptor)
{
	for (int i = 0; i < requests_.size(); ++i) {
		const std::pair<std::string, std::string> requesterAndCommand = getRequesterAndCommand(requests_[i]);
		if (requesterAndCommand.second == "sendMessage") {
			performSendMessage(socketDescriptor, requests_[i]);
		} else if (requesterAndCommand.second == "auth") {
			performAuth(socketDescriptor, requests_[i]);
		} else if (requesterAndCommand.second == "fetchMessages") {
			performFetchMessages(socketDescriptor, requests_[i]);
		} else if (requesterAndCommand.second == "fetchKeys") {
			performFetchKeys(socketDescriptor, requests_[i]);
		} else if (requesterAndCommand.second == "initialMessage") {
			performSendInitialMsg(socketDescriptor, requests_[i]);
		} else if (requesterAndCommand.second == "createAccount") {
			tryCreateAccount(socketDescriptor, requests_[i]);
		}
	}

	while (isFileLocked(RESPONSE_FILE_PATH)) {
		// waiting
	}

	lockFile(RESPONSE_FILE_PATH);

	std::ofstream resFile(RESPONSE_FILE_PATH, std::ofstream::out | std::ofstream::app);

	for (int i = 0; i < responses_.size(); ++i) {
		resFile << responses_[i] << std::endl;
	}

	resFile.close();
	requests_.clear();
	responses_.clear();
	unlockFile(RESPONSE_FILE_PATH);
}

void Server::performSendMessage(int socketDescriptor, const std::string &req) {
	int delim = req.find_first_of(';');
	std::string sender = req.substr(0, delim);
	std::string tmp = req.substr(delim + 1);
	delim = tmp.find_first_of(';');
	std::string command = tmp.substr(0, delim);
	tmp = tmp.substr(delim + 1);
	delim = tmp.find_first_of(';');
	std::string reciever = tmp.substr(0, delim);
	std::string text = tmp.substr(delim + 1);

	int sendIndex = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == sender) {
			sendIndex = i;
			break;
		}
	}
	if (sendIndex == -1) {
		throw std::runtime_error("what the fuck did just happen?!");
	}
	int recIndex = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == reciever) {
			recIndex = i;
			break;
		}
	}
	if (recIndex == -1) {
		responses_.emplace_back(users_[sendIndex].name() + ";sendMessage;Reciever does not exist");
		return;
	}

	Mess theMessage;
	theMessage.set_sender(sender);
	theMessage.set_reciever(reciever);
	theMessage.set_text(text);

	std::string stringified;
	theMessage.SerializeToString(&stringified);

	users_[recIndex].add_messages(stringToHex(stringified));
	users_[sendIndex].add_messages(stringToHex(stringified));

	std::string response = users_[sendIndex].name() + ";sendMessage;Message sent";
	send(socketDescriptor, response.c_str(), response.size(), 0);
	//responses_.emplace_back(users_[sendIndex].name() + ";sendMessage;Message sent");
}

void Server::performAuth(int socketDescriptor, const std::string &req) {
	int delim = req.find_first_of(';');
	std::string login = req.substr(0, delim);
	std::string tmp = req.substr(delim + 1);
	delim = tmp.find_first_of(';');
	std::string command = tmp.substr(0, delim);
	std::string pwd = tmp.substr(delim + 1);

	int index = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == login) {
			if (users_[i].pwdhash() == pwd) {
				std::string line;
				users_[i].SerializeToString(&line);
				std::stringstream usr;
				usr << stringToHex(line);
				std::string response = users_[i].name() + ";auth;" + usr.str();
				int socketIndex = findSocketIndexFromSocket(socketDescriptor);
				//sockets_[socketIndex].second = users_[i].name();
				send(socketDescriptor, response.c_str(), response.size(), 0);
			} else {
				std::string response = users_[i].name() + ";auth;Wrong password";
				send(socketDescriptor, response.c_str(), response.size(), 0);
			}
			index = i;
		}
	}
	if (index == -1) {
		// easy in http, hard in file-based communication
	}
}

void Server::performFetchMessages(int socketDescriptor, const std::string &req) {
	int delim = req.find_first_of(';');
	std::string name = req.substr(0, delim);

	int index = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == name) {
			index = i;
			break;
		}
	}

	auto messages = users_[index].messages();
	for (const auto &it : messages) {
		std::string response = users_[index].name() + ";fetchMessages;" + it;
		send(socketDescriptor, response.c_str(), response.size(), 0);
		//responses_.push_back(response);
	}
}

void Server::performSendInitialMsg(int socketDescriptor, const std::string &req) {
	int delim = req.find_first_of(';');
	std::string name = req.substr(0, delim);
	std::string tmp = req.substr(delim + 1);
	delim = tmp.find_first_of(';');
	tmp = tmp.substr(delim + 1);
	delim = tmp.find_first_of(';');
	std::string rec = tmp.substr(0, delim);
	std::string text = tmp.substr(delim + 1);
	std::string response = rec + ";initialMessage;" + name + ";" + text;

	int recieverSocketIndex = findSocketIndexFromName(rec);
	if (recieverSocketIndex == -1) {
		std::cerr << "Reciever does not exist" << std::endl;
		return;
	}

	//int recieverSocket = sockets_[recieverSocketIndex].first;
	int recieverSocket = 0;

	send(recieverSocket, response.c_str(), response.size(), 0);
	//responses_.push_back(response);
}

void Server::performFetchKeys(int socketDescriptor, const std::string &req) {
	int delim = req.find_first_of(';');
	std::string name = req.substr(0, delim);
	std::string tmp = req.substr(delim + 1);
	delim = tmp.find_first_of(';');
	std::string keysOwner = tmp.substr(delim + 1);
	int index = -1;
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == keysOwner) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		std::string response = name + ";fetchKeys;Error";
		send(socketDescriptor, response.c_str(), response.size(), 0);
		//responses_.push_back(response);
		return;
	}
	std::stringstream response;
	response << name + ";fetchKeys;" + keysOwner + ";";
	response << users_[index].publicik() << ";";
	response << users_[index].publicpk() << ";";
	response << users_[index].signedpk() << ";";

	//TODO: take only one and delete it from server
	auto oneTimes = users_[index].onetimepublic();
	for (const auto &it : oneTimes) {
		response << it;
		break;
	}
	std::string responseStr = response.str();
	std::cout << "sending keys: " << responseStr << std::endl;
	send(socketDescriptor, responseStr.c_str(), responseStr.size(), 0);
	//responses_.push_back(response.str());
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
		std::cout << "load user with signature:" << user.signedpk() << std::endl;
		users_.push_back(user);
	}
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

		std::cout << "writting user with signature:" << users_[i].signedpk() << std::endl;

		users_[i].SerializeToString(&line);

		usersFile << stringToHex(line);
	}
	usersFile.close();
}

const std::pair<std::string, std::string> Server::getRequesterAndCommand(const std::string &request) {
	int delim = request.find_first_of(';');
	const std::string requester = request.substr(0, delim);
	std::string rest = request.substr(delim + 1);
	delim = rest.find_first_of(';');
	const std::string command = rest.substr(0, delim);
	return std::make_pair(requester, command);
}

void Server::runServer() {
	//sockets_.resize(0);
	std::cout << "sockets_ size " << sockets_.size() << std::endl;
	for (int i = 0; i < 30; ++i) {
		auto newPair = std::make_pair(0, "");
		//sockets_.push_back(newPair);
		sockets_.push_back(0);
	}
	std::cout << "sockets_ size " << sockets_.size() << std::endl;
	// for (int i = 0; i < 30; ++i) {
	// 	std::cout << "watever" << std::endl;
	// 	std::cout << "socket: " << sockets_[i].first << ", " << sockets_[i].second << std::endl;
	// }
	int opt = 1;
    int master_socket , addrlen , new_socket,
          max_clients = 30 , activity, i , valread , socketDescriptor;
    int maxSocketDescriptor;
    struct sockaddr_in address;

    char buffer[2049];  //data buffer of 1K

    //std::vector<int> client_socket(30, 0);

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
            //socketDescriptor = client_socket[i];
			std::cout << "pica" << std::endl;
			socketDescriptor = sockets_[i];//.first;
			std::cout << "descriptor: " << socketDescriptor << std::endl;

            //if valid socket descriptor then add to read list
            if(socketDescriptor > 0)
                FD_SET( socketDescriptor , &readfds);

            //highest file descriptor number, need it for the select function
            if(socketDescriptor > maxSocketDescriptor)
                maxSocketDescriptor = socketDescriptor;
			std::cout << "pica after" << std::endl;
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
                // if( client_socket[i] == 0 )
                // {
                //     client_socket[i] = new_socket;
                //     std::cout << "Socket added " << i << std::endl;
                //     break;
                // }
				// if( sockets_[i].first == 0 )
                // {
                //     sockets_[i].first = new_socket;
                //     std::cout << "Socket added " << i << std::endl;
                //     break;
                // }
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
            //socketDescriptor = client_socket[i];
			socketDescriptor = sockets_[i];//.first;

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
                    //client_socket[i] = 0;
					//sockets_[i].first = 0;
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
                }
            }
        }
    }
}
