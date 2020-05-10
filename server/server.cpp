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

void Server::checkRequests()
{
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
}

void Server::processRequests()
{
	Mess message;
	for (int i = 0; i < requests_.size(); ++i) {
		if (requests_[i] == "") {
			continue;
		}

		message.ParseFromString(hexToString(requests_[i]));

		switch (message.type()) {
		case Mess::MESSAGE:
			performSendMessage(requests_[i]);
			break;
		case Mess::AUTH:
			performAuth(requests_[i]);
			break;
		case Mess::FETCH_MESSAGES:
			performFetchMessages(requests_[i]);
			break;
		case Mess::FETCH_KEYS:
			performFetchKeys(requests_[i]);
			break;
		case Mess::INIT_MESSAGE:
			performSendInitialMsg(requests_[i]);
			break;
		case Mess::CREATE_ACCOUNT:
			performCreateAccount(requests_[i]);
			break;
		default:
			break;
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

void Server::performSendMessage(const std::string &req) {
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
		responses_.emplace_back(stringToHex(serialized));
		return;
	}
	message.set_error(false);

	message.SerializeToString(&serialized);

	// add messages to corresponding users
	users_[recIndex].add_messages(stringToHex(serialized));
	users_[sendIndex].add_messages(stringToHex(serialized));

	// write response for sender
	response.SerializeToString(&serialized);
	responses_.emplace_back(stringToHex(serialized));
}

void Server::performAuth(const std::string &req) {
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
	responses_.push_back(stringToHex(serialized));


	//std::cout << "OK" << std::endl;
}

void Server::performFetchMessages(const std::string &req) {
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
		responses_.push_back(stringToHex(serialized));
		fetched.clear_textcontent();
	}

	//std::cout << "DEBUG: messages fetched" << std::endl;
}

void Server::performSendInitialMsg(const std::string &req) {
	responses_.push_back(req);
}

void Server::performFetchKeys(const std::string &req) {
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
		responses_.push_back(stringToHex(serialized));
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
	responses_.push_back(stringToHex(serialized));
	//std::cout << "OK" << std::endl;
}

void Server::performCreateAccount(const std::string &req) {
	//std::cout << "creating account ... ";
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
			responses_.push_back(stringToHex(serialized));
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
	responses_.push_back(stringToHex(serialized));

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
