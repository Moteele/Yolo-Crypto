#include "functions.hpp"
#include <google/protobuf/stubs/common.h>
#include "catch.hpp"
#include <cstring>
#include <vector>
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>

TEST_CASE("string-hex conversions")
{
	// test vector: std::pair<string, hex>
	std::vector<std::pair<std::string, std::string>> test;
	test.emplace_back("ABCDabcd012342",
			  "4142434461626364303132333432");
	test.emplace_back("aldjlkjjvjvjkkkalkl1kl009902jjvm,amv../akijj",
			  "616c646a6c6b6a6a766a766a6b6b6b616c6b6c316b6c3030393930326a6a766d2c616d762e2e2f616b696a6a");
	test.emplace_back("askdjk //'[\\kljkjk`kkj",
			  "61736b646a6b202f2f275b5c6b6c6a6b6a6b606b6b6a");
	test.emplace_back("qwertyuiopasdfghjklzxcvbnm",
			  "71776572747975696f706173646667686a6b6c7a786376626e6d");
	test.emplace_back("}{:::  JJJKKkllkk10909/.,",
			  "7d7b3a3a3a20204a4a4a4b4b6b6c6c6b6b31303930392f2e2c");
	test.emplace_back("\\][|}{=-+_/.,?><';\":!@#$%^&*()`~1234567890",
			  "5c5d5b7c7d7b3d2d2b5f2f2e2c3f3e3c273b223a21402324255e262a2829607e31323334353637383930");

	std::vector<std::pair<std::string, std::string>> random;
	std::string string;
	std::stringstream hex;
	unsigned char c;
	for (int j = 0; j < 64; ++j) {
		for (int i = 0; i < 256; ++i) {
			c = std::rand() % 256;
			string.push_back(static_cast<unsigned>(c));
			hex << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(c);
		}
		random.emplace_back(string, hex.str());
	}

	SECTION("hexToString")
	{
		std::string str;

		for (auto &el : test) {
			str = hexToString(el.second);
			REQUIRE(str == el.first);
			REQUIRE(std::memcmp(&str[0], &el.first[0], el.first.size()) == 0);
		}
	}

	SECTION("stringToHex")
	{
		std::string str;

		for (auto &el : test) {
			str = stringToHex(el.first);
			REQUIRE(str == el.second);
			REQUIRE(std::memcmp(&str[0], &el.second[0], el.second.size()) == 0);
		}
	}

	SECTION("hexToString random")
	{
		std::string str;

		for (auto &el : random) {
			str = hexToString(el.second);
			REQUIRE(str == el.first);
			REQUIRE(std::memcmp(&str[0], &el.first[0], el.first.size()) == 0);
		}
	}

	SECTION("stringToHex random")
	{
		std::string str;

		for (auto &el : random) {
			str = stringToHex(el.first);
			REQUIRE(str == el.second);
			REQUIRE(std::memcmp(&str[0], &el.second[0], el.second.size()) == 0);
		}
	}

	google::protobuf::ShutdownProtobufLibrary();
}
