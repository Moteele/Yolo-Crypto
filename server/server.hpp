#include <google/protobuf/stubs/common.h>
#define LIB_PATH "./libs/libmbedcrypto.so"

#ifndef SERVER_HPP
#define SERVER_HPP

#include "message.pb.h"
#include "../utils/constants.hpp"
#include "../utils/functions.hpp"
#include "../utils/mess.pb.h"
#include "../utils/userAcc.pb.h"
#include "../utils/util.hpp"

#include <arpa/inet.h>    //close
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <ostream>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <unistd.h>   //close
#include <utility>
#include <vector>


class Account
{
private:
	const unsigned int id_;
	std::string login_;
	std::string displayName_;
	std::set<unsigned int> contacts_;
	std::string pwdHash_;

public:
	/**
	 * Constructor
	 * @param id			unique id
	 * @param login			login email
	 * @param pwdHash		hash of password
	 * @param displayName		name to be displayed
	 */
	Account(unsigned int id, const std::string &login, const std::string &pwdHash, const std::string &displayName) :
		id_(id), login_(login), displayName_(displayName), pwdHash_(pwdHash) {}

	/**
	 * getter
	 * @return	id_ value
	 */
	unsigned int getId() { return id_; }

	/**
	 * getter
	 * @return	login_ value
	 */
	const std::string &getLogin() const { return login_; }

	/**
	 * getter
	 * @return	displayName_ value
	 */
	const std::string &getDisplayName() const { return displayName_; }

	/**
	 * getter
	 * @return	contacts_ value
	 */
	const std::set<unsigned int> &getContacts() const { return contacts_; }

	/**
	 * getter
	 * @return	pwdHash_ value
	 */
	const std::string &getPwdHash() const { return pwdHash_; }



	/**
	 * setter
	 * @param login		login to be set
	 */
	void setLogin(std::string &login) { login_ = login; }

	/**
	 * setter
	 * @param name		diplayName to be set
	 */
	void setDisplayName(std::string &name) { displayName_ = name; }

	/**
	 * adds a contact
	 * @param id		id of an account
	 */
	void addContact(unsigned int contact) { contacts_.insert(contact); }

	/**
	 * removes a contact
	 * @param id		id of an account
	 */
	void removeContact(unsigned int id) { contacts_.erase(id); }

	/**
	 * setter
	 * @param hash		pwdHash to be set
	 */
	void setPwdHash(std::string &hash) { pwdHash_ = hash; }

	/**
	 * prints account info
	 * @param os		output stream to write into
	 */
	void print(std::ostream &os) const;


};

class Server
{
private:
	unsigned int id_ = 1;
	std::map<unsigned int, Account> database_;
	std::set<unsigned int> online_;
	void *cryptoLib_;

	// temporary atribute just for recieving requests
	// TODO: delete later
	std::vector<std::string> requests_ = {};
	std::vector<std::string> responses_ = {};

	std::vector<userAcc> users_ = {};

	std::vector<int> sockets_ = {};

public:
	/**
	 * Constructor
	 * @param database		std::set<Account>
	 */
	Server(std::map<unsigned int, Account> &database) : database_(database)
	{
		//cryptoLib_ = dlopen(LIB_PATH, RTLD_LAZY);

		//if (!cryptoLib_) {
		//	std::cerr << dlerror();
		//	throw std::runtime_error(dlerror());
		//}
	}

	Server() {}

	/**
	 * Destructor
	 */
	~Server()
	{
		google::protobuf::ShutdownProtobufLibrary();
	}

	/**
	 * getter
	 * @return	id_ value
	 */
	const unsigned int getId() const { return id_; }

	/**
	 * getter
	 * @return	database_ value
	 */
	const std::map<unsigned int, Account> &getDatabase() const { return database_; }

	/**
	 * non const getter, for testing purposes
	 * @return	database_ value
	 */
	std::map<unsigned int, Account> &getDatabase() { return database_; }

	/**
	 * getter
	 * @return	online_ value
	 */
	const std::set<unsigned int> &getOnline() const { return online_; }

	/**
	 * setter
	 * @param id		id to be set
	 */
	void setId(unsigned int id) { id_ = id; }



	/**
	 * creates an account
	 * @param login		email, must be unique
	 * @param name		visible name
	 * @param pwdHash	hash of password
	 * @return		0 if login is not unique, id of new account otherwise
	 */
	unsigned int createAccount(const std::string &login, const std::string &pwdHash, const std::string &name);

	/**
	 * deletes an account
	 * @param id		id of account to be deleted
	 * @return		true if the account was deleted, false otherwise
	 */
	bool deleteAccount(unsigned int id);

	/**
	 * signs in a registered user that is not already signed in
	 * @param login		login email
	 * @param pwdHash	password hash
	 * @param challenge	random number challenge
	 * @return		true if successful, false otherwise
	 */
	bool signIn(const std::string &login, const std::string &pwdHash, const unsigned int challenge);

	/**
	 * signs out an user that is online
	 * @param id		id of an user
	 * @return		true if the user has been signed out, false otherwise
	 */
	bool signOut(unsigned int id);

	/**
	 * list all registered users
	 * @param os		stream to write into
	 */
	void listUsers(std::ostream &os) const;

	/**
	 * list online users
	 * @param os		stream to write into
	 */
	void listOnline(std::ostream &os) const;

	/**
	 * test function
	 */
	void test();


	void checkRequests();

	void processRequests(int socketDescriptor);

	void loadUsers();

	void writeUsers();

	const std::pair<std::string, std::string> getRequesterAndCommand(const std::string &request);

	void performSendMessage(int socketDescriptor, const std::string &req);

	void performAuth(int socketDescriptor, const std::string &req);

	void performFetchMessages(int socketDescriptor, const std::string &req);

	void performFetchKeys(int socketDescriptor, const std::string &req);

	void performSendInitialMsg(int socketDescriptor, const std::string &req);

	void readRequest(const std::string &req);

	void runServer();

	void performCreateAccount(int socketDescriptor, const std::string &req);
};

#endif // SERVER_HPP
