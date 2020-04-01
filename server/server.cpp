#include "server.hpp"
#include "util.hpp"
#include <sstream>
#include <cstring>
#include "message.pb.h"
#include <filesystem>
#include <fstream>


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

	Util::hash512(dbHash, dbHash.size(), hash, cryptoLib_);

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
	Message msg;
	Message msg2;
	msg.set_recid(42);
	msg.set_senid(32);
	msg.clear_textcontent();
	msg.set_textcontent("this is some text");
	msg.add_kdfkeys("key0");
	msg.add_kdfkeys("key1");
	msg.add_kdfkeys("key2");
	msg.add_kdfkeys("key3");

	std::cout << msg.DebugString() << std::endl;

	std::string str;

	msg.SerializeToString(&str);

	std::cout << "serialized message = " << str << std::endl;

	msg2.ParseFromString(str);

	std::cout << std::endl;

	std::cout << msg2.DebugString() << std::endl;
	std::cout << msg2.textcontent() << std::endl;
}

void Server::checkRequests()
{
    std::string path = "TODO";
	std::vector<std::string> requestFiles;
	std::vector<std::string> locks;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
		if (entry.path().string().substr(entry.path().string().size() - -4, 4) == "lock") {
			locks.push_back(entry.path().string()); // TODO get only the filename
		} else {
			requestFiles.push_back(entry.path().string()); //TODO get only the filename
		}
	}
    
	for (int i = 0; i < requestFiles.size(); ++i) {
		std::string req = requestFiles[i]; //TODO get rid of .txt
		bool locked = false;
		for (const auto & lock : locks) {
			if (lock == req) {
				locked = true;
				break;
			}
		}
		if (locked) {
			continue;
		}
		std::ofstream LockFile(path + req + ".lock"); // lock the file
		LockFile.close();

		std::ifstream ReqFile(path + req + ".txt"); // open requests
		std::string line;
		while (std::getline(ReqFile, line)) {
			requests_.emplace_back(req, line); // store request
		}
		ReqFile.close();
		std::ofstream EraseReqFile(path + req + ".txt", std::ofstream::out | std::ofstream::trunc);
		EraseReqFile.close();

		std::filesystem::remove(path + req + ".lock"); // unlock the file
	}
}

void Server::processRequests()
{
	std::string path = "TODO";
	std::vector<std::string> responseFiles;
	std::vector<std::string> locks;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
		if (entry.path().string().substr(entry.path().string().size() - -4, 4) == "lock") {
			locks.push_back(entry.path().string()); // TODO get only the filename
		} else {
			responseFiles.push_back(entry.path().string()); //TODO get only the filename
		}
	}

	std::vector<int> solvedIndex;
	for (int i = 0; i < responseFiles.size(); ++i) {
		std::string res = responseFiles[i];
		bool locked = false;
		for (const auto & lock : locks) {
			if (lock == res) {
				locked = true;
				break;
			}
		}
		if (locked) {
			continue;
		}

		std::ofstream LockFile(path + res + ".lock"); // lock the file
		LockFile.close();

		std::ofstream ResFile(path + res + ".txt", std::ofstream::out | std::ofstrea::app); // open the response file

		for (int i = 0; i < requests_.size(); ++i) {
			if (requests_[i].first == res) {
				int delim = requests_[i].second.find_first_of(';');
				std::string command = requests_[i].second.substr(0, delim);
				std::string arg = requests_[i].second.substr(delim);

				if (command == "sendMessage") {
					ResFile << "recieveMessage;" << arg << std::endl;
				}

				solvedIndex.push_back(i);
			}
		}

		ResFile.close();
		std::filesystem::remove(path + res + ".lock");
	}

	for (int i = solvedIndex.size() - 1; i > -1; --i) { // remove solved requests from requests_
		requests_.erase(requests_.begin() + i);
	}
}













