#include <string>
#include "../libs/catch.hpp"
#include "../libs/util.hpp"
#include "server.hpp"

TEST_CASE("basic server tests")
{
	// init

	std::map<unsigned int, Account> db;
	db.emplace(std::make_pair(1, Account(1, "1abc@def.org", "1hashpwd", "1name")));
	db.emplace(std::make_pair(2, Account(2, "2abc@def.org", "2hashpwd", "2name")));
	db.emplace(std::make_pair(3, Account(3, "3abc@def.org", "3hashpwd", "3name")));
	Server server(db);
	server.setId(4);


	//void *lib = dlopen(LIB_PATH, RTLD_LAZY);

	SECTION("playground")
	{
		std::cout << "=============Playground=============" << std::endl;
		server.test();
		std::cout << "=============Playground=============" << std::endl;
	}


	SECTION("Registration")
	{
		REQUIRE(server.createAccount("4abc@def.org", "4hashpwd", "4name") == 4);
		REQUIRE(server.createAccount("4abc@def.org", "5hashpwd", "5name") == 0);


		std::string id;

		for (unsigned int i = 5; i < 120; ++i) {
			id = std::to_string(i);
			REQUIRE(server.createAccount(id + "abc@def.org", id + "hashpwd", id + "name") == i);
		}

		REQUIRE(server.getDatabase().size() == 119);

		for (unsigned int i = 1; i < 120; ++i) {
			id = std::to_string(i);
			REQUIRE(server.createAccount(id + "abc@def.org", id + "hashpwd", id + "name") == 0);
		}

		REQUIRE(server.getDatabase().size() == 119);

	}

	SECTION("Remove account")
	{
		REQUIRE(server.deleteAccount(1));
		REQUIRE_FALSE(server.deleteAccount(1));

		REQUIRE(server.getDatabase().size() == 2);

		std::string id;

		for (unsigned int i = 4; i < 120; ++i) {
			id = std::to_string(i);
			server.getDatabase().emplace(std::make_pair(i, Account(i, id + "abc@def.org", id + "hashpwd", id + "name")));
		}

		REQUIRE(server.getDatabase().size() == 118);

		for (unsigned int i = 4; i < 120; ++i) {
			REQUIRE(server.deleteAccount(i));
		}

		for (unsigned int i = 4; i < 120; ++i) {
			REQUIRE_FALSE(server.deleteAccount(i));
		}

		REQUIRE(server.getDatabase().size() == 2);
	}

	SECTION("Login/logout")
	{
		REQUIRE(server.getOnline().size() == 0);
		REQUIRE_FALSE(server.signIn("1abc@def.org", "1hashpwd", 42));

		REQUIRE(server.getOnline().size() == 0);

		server.listUsers(std::cout);
		server.listOnline(std::cout);

		std::string hash;
		std::string toHash;

		std::string id;
		for (unsigned int i = 4; i < 120; ++i) {
			id = std::to_string(i);
			Util::hash512(id + "hashpwd", 7 + id.size(), hash);
			server.getDatabase().emplace(std::make_pair(i, Account(i, id + "abc@def.org", hash, id + "name")));
			hash.clear();
		}


		// login
		for (unsigned int i = 4; i < 120; ++i) {
			id = std::to_string(i);
			toHash = id + "hashpwd";

			Util::hash512(toHash, toHash.size(), hash);
			toHash = hash;
			Util::hash512(toHash + "42", toHash.size() + 2, hash);
			REQUIRE(server.signIn(id + "abc@def.org", hash, 42));
			REQUIRE_FALSE(server.signIn(id + "abc@def.org", hash, 42));
			hash.clear();
		}

		REQUIRE(server.getOnline().size() == 116);

		// logout
		for (unsigned int i = 4; i < 120; ++i) {
			REQUIRE(server.signOut(i));
		}

		REQUIRE(server.getOnline().size() == 0);

		for (unsigned int i = 4; i < 120; ++i) {
			REQUIRE_FALSE(server.signOut(i));
		}

		REQUIRE(server.getOnline().size() == 0);

		// login wrong password
		for (unsigned int i = 4; i < 120; ++i) {
			id = std::to_string(i);
			REQUIRE_FALSE(server.signIn(id + "abc@def.org", id + "WRONG", 42));
		}
	}


	//dlclose(lib);
}
