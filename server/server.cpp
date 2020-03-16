#include "server.hpp"

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

bool Server::signIn(const std::string &login, const std::string &pwdHash)
{
	unsigned int id = 0;
	for (auto &acc : database_) {
		if (acc.second.getLogin() == login && acc.second.getPwdHash() == pwdHash) {
			id = acc.first;
			break;
		}
	}

	if (id == 0 || online_.find(id) != online_.end()) {
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
