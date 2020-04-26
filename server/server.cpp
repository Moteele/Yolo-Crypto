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
			std::cout << "setting signature:" << value << std::endl;
			continue;
		}
		if (command == "privateOnetime") {
			newUser.add_onetimeprivate(value);
			continue;
		}
		if (command == "publicOnetime") {
			std::cout << "publicOneTime " << value << std::endl;
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

	std::cout << "signature:" << users_[index].signedpk() << "\n";
	//TODO: take only one and delete it from server
	auto oneTimes = users_[index].onetimepublic();
	std::cout << "len of oneTimes:" << oneTimes.size() << std::endl;
	for (const auto &it : oneTimes) {
		std::cout << "oneTime:" << it;
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

void fillRawKey(const std::vector<uint8_t> &key, unsigned char *rawKey, int rawKeySize) {
	if (key.size() != rawKeySize) {
		std::cerr << "Keys do not have the same length!" << std::endl;
		return;
	}
	for (int i = 0; i < rawKeySize; ++i) {
		*rawKey = key[i];
		++rawKey;
	}
}

void Server::testSecretCalc() {
	Key bobIdentity;
	Key bobSignedPrekey;
	Key bobOneTimePrekey;
	Key aliceIdentity;
	Key aliceEphemeralKey;

	unsigned char bobIdentityPrivate[32];
	unsigned char bobIdentityPublic[32];
	unsigned char bobSignedPrekeyPrivate[32];
	unsigned char bobSignedPrekeyPublic[32];
	unsigned char bobOneTimePrekeyPrivate[32];
	unsigned char bobOneTimePrekeyPublic[32];

	unsigned char aliceIdentityPrivate[32];
	unsigned char aliceIdentityPublic[32];
	unsigned char aliceEphemeralPrivate[32];
	unsigned char aliceEphemeralPublic[32];

	fillRawKey(bobIdentity.getPrivateKey(), bobIdentityPrivate, 32);
	fillRawKey(bobIdentity.getPublicKey(), bobIdentityPublic, 32);
	fillRawKey(bobSignedPrekey.getPrivateKey(), bobSignedPrekeyPrivate, 32);
	fillRawKey(bobSignedPrekey.getPublicKey(), bobSignedPrekeyPublic, 32);
	fillRawKey(bobOneTimePrekey.getPrivateKey(), bobOneTimePrekeyPrivate, 32);
	fillRawKey(bobOneTimePrekey.getPublicKey(), bobOneTimePrekeyPublic, 32);

	fillRawKey(aliceIdentity.getPrivateKey(), aliceIdentityPrivate, 32);
	fillRawKey(aliceIdentity.getPublicKey(), aliceIdentityPublic, 32);
	fillRawKey(aliceEphemeralKey.getPrivateKey(), aliceEphemeralPrivate, 32);
	fillRawKey(aliceEphemeralKey.getPublicKey(), aliceEphemeralPublic, 32);

	// ALICE
	// DH1
	EVP_PKEY *fstA = EVP_PKEY_new();
    fstA = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceIdentityPrivate, 32);
    EVP_PKEY *sndA = EVP_PKEY_new();
    sndA = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobSignedPrekeyPublic, 32);
    size_t ssizeA;
    unsigned char DH1A[32];
    Util::ecdh(fstA, sndA, DH1A, &ssizeA);
	std::cout << "Alice DH1" << std::endl;
	for (int i = 0; i < 32; ++i) {
		std::cout << DH1A[i];
	}
	std::cout << std::endl;

	// DH2
	EVP_PKEY *fst2A = EVP_PKEY_new();
    fst2A = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceEphemeralPrivate, 32);
    EVP_PKEY *snd2A = EVP_PKEY_new();
    snd2A = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobIdentityPublic, 32);
    size_t ssize2A;
    unsigned char DH2A[32];
    Util::ecdh(fst2A, snd2A, DH2A, &ssize2A);

	// DH3
	EVP_PKEY *fst3A = EVP_PKEY_new();
    fst3A = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceEphemeralPrivate, 32);
    EVP_PKEY *snd3A = EVP_PKEY_new();
    snd3A = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobSignedPrekeyPublic, 32);
    size_t ssize3A;
    unsigned char DH3A[32];
    Util::ecdh(fst3A, snd3A, DH3A, &ssize3A);

	// DH4
	EVP_PKEY *fst4A = EVP_PKEY_new();
    fst4A = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceEphemeralPrivate, 32);
    EVP_PKEY *snd4A = EVP_PKEY_new();
    snd4A = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobOneTimePrekeyPublic, 32);
    size_t ssize4A;
    unsigned char DH4A[32];
    Util::ecdh(fst4A, snd4A, DH4A, &ssize4A);

	// SK
	unsigned char SKA[128];
	for (int i = 0; i < 32; ++i) {
		SKA[i] = DH1A[i];
	}
	for (int i = 32; i < 64; ++i) {
		SKA[i] = DH2A[i - 32];
	}
	for (int i = 64; i < 96; ++i) {
		SKA[i] = DH3A[i - 64];
	}
	for (int i = 96; i < 128; ++i) {
		SKA[i] = DH4A[i - 96];
	}
	unsigned char sharedSecretA[32];
	size_t sharedSecretALen = 32;
	int result = Util::kdf(SKA, 128, sharedSecretA, &sharedSecretALen);
	std::cout << "Secret Length: " << sharedSecretALen << std::endl;
	std::cout << "Alice Shared Secret:" << std::endl;
	for (int i = 0; i < sharedSecretALen; ++i) {
		std::cout << sharedSecretA[i];
	}
	std::cout << std::endl;


	// BOB
	// DH1
	EVP_PKEY *fstB = EVP_PKEY_new();
    fstB = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceIdentityPublic, 32);
    EVP_PKEY *sndB = EVP_PKEY_new();
    sndB = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobSignedPrekeyPrivate, 32);
    size_t ssizeB;
    unsigned char DH1B[32];
    Util::ecdh(fstB, sndB, DH1B, &ssizeB);
	std::cout << "Bob DH1" << std::endl;
	for (int i = 0; i < 32; ++i) {
		std::cout << DH1B[i];
	}
	std::cout << std::endl;

	// DH2
	EVP_PKEY *fst2B = EVP_PKEY_new();
    fst2B = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceEphemeralPublic, 32);
    EVP_PKEY *snd2B = EVP_PKEY_new();
    snd2B = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobIdentityPrivate, 32);
    size_t ssize2B;
    unsigned char DH2B[32];
    Util::ecdh(fst2B, snd2B, DH2B, &ssize2B);

	// DH3
	EVP_PKEY *fst3B = EVP_PKEY_new();
    fst3B = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceEphemeralPublic, 32);
    EVP_PKEY *snd3B = EVP_PKEY_new();
    snd3B = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobSignedPrekeyPrivate, 32);
    size_t ssize3B;
    unsigned char DH3B[32];
    Util::ecdh(fst3B, snd3B, DH3B, &ssize3B);

	// DH4
	EVP_PKEY *fst4B = EVP_PKEY_new();
    fst4B = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, aliceEphemeralPublic, 32);
    EVP_PKEY *snd4B = EVP_PKEY_new();
    snd4B = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, bobOneTimePrekeyPrivate, 32);
    size_t ssize4B;
    unsigned char DH4B[32];
    Util::ecdh(fst4B, snd4B, DH4B, &ssize4B);

	// SK
	unsigned char SKB[128];
	for (int i = 0; i < 32; ++i) {
		SKB[i] = DH1B[i];
	}
	for (int i = 32; i < 64; ++i) {
		SKB[i] = DH2B[i - 32];
	}
	for (int i = 64; i < 96; ++i) {
		SKB[i] = DH3B[i - 64];
	}
	for (int i = 96; i < 128; ++i) {
		SKB[i] = DH4B[i - 96];
	}
	unsigned char sharedSecretB[32];
	size_t sharedSecretBLen = 32;
	int result2 = Util::kdf(SKB, 128, sharedSecretB, &sharedSecretBLen);
	std::cout << "Secret Length: " << sharedSecretBLen << std::endl;
	std::cout << "Bob Shared Secret:" << std::endl;
	for (int i = 0; i < sharedSecretBLen; ++i) {
		std::cout << sharedSecretB[i];
	}
	std::cout << std::endl;
}
