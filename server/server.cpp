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
}

int Server::tryCreateAccount(const std::string &name) {
	for (int i = 0; i < users_.size(); ++i) {
		if (users_[i].name() == name) {
			// duplicit name
			return -1;
		}
	}
	std::string path = "tmp_files/req/" + name;
	std::ifstream req(path);

	userAcc newUser;
	std::string line;
	newUser.set_name(name);
	newUser.set_id(users_.size());
	while (std::getline(req, line)) {
		int delim = line.find_first_of(':');
		std::string command = line.substr(0, delim);
		std::string value = line.substr(delim + 1);
		if (command == "password") {
			newUser.set_pwdhash(value);
			continue;
		}
		if (command == "privateId") {
			newUser.set_privateik(value);
			continue;
		}
		if (command == "publicId") {
			newUser.set_publicik(value);
			continue;
		}
		if (command == "privateSignedPrekey") {
			newUser.set_privatepk(value);
			continue;
		}
		if (command == "publicSignedPrekey") {
			newUser.set_publicpk(value);
			continue;
		}
		if (command == "prekeySignature") {
			newUser.set_signedpk(value);
			continue;
		}
		if (command == "privateOnetime") {
			newUser.add_onetimeprivate(value);
			continue;
		}
		if (command == "publicOnetime") {
			newUser.add_onetimepublic(value);
			continue;
		}
	}
	users_.push_back(newUser);
	req.close();
	std::remove(path.c_str());
	return 0;
}

void Server::processRequests()
{
	for (int i = 0; i < requests_.size(); ++i) {
		const std::pair<std::string, std::string> requesterAndCommand = getRequesterAndCommand(requests_[i]);
		if (requesterAndCommand.second == "sendMessage") {
			performSendMessage(requests_[i]);
		} else if (requesterAndCommand.second == "auth") {
			performAuth(requests_[i]);
		} else if (requesterAndCommand.second == "fetchMessages") {
			performFetchMessages(requests_[i]);
		} else if (requesterAndCommand.second == "fetchKeys") {
			performFetchKeys(requests_[i]);
		} else if (requesterAndCommand.second == "initialMessage") {
			performSendInitialMsg(requests_[i]);
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

	responses_.emplace_back(users_[sendIndex].name() + ";sendMessage;Message sent");
}

void Server::performAuth(const std::string &req) {
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
				responses_.emplace_back(users_[i].name() + ";auth;" + usr.str());
			} else {
				responses_.emplace_back(users_[i].name() + ";auth;Wrong password");
			}
			index = i;
		}
	}
	if (index == -1) {
		// easy in http, hard in file-based communication
	}
}

void Server::performFetchMessages(const std::string &req) {
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
		responses_.push_back(response);
	}
}

void Server::performSendInitialMsg(const std::string &req) {
	int delim = req.find_first_of(';');
	std::string name = req.substr(0, delim);
	std::string tmp = req.substr(delim + 1);
	delim = tmp.find_first_of(';');
	tmp = tmp.substr(delim + 1);
	delim = tmp.find_first_of(';');
	std::string rec = tmp.substr(0, delim);
	std::string text = tmp.substr(delim + 1);
	std::string response = rec + ";initialMessage;" + text;
	responses_.push_back(response);
}

void Server::performFetchKeys(const std::string &req) {
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
		responses_.push_back(response);
		return;
	}
	std::stringstream response;
	response << name + ";fetchKeys;";
	response << users_[index].publicik() << ";";
	response << users_[index].publicpk() << ";";
	response << users_[index].signedpk() << ";";

	//TODO: take only one and delete it from server
	auto oneTimes = users_[index].onetimepublic();
	for (const auto &it : oneTimes) {
		response << it;
		break;
	}
	responses_.push_back(response.str());
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
