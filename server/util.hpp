#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <dlfcn.h>
#include <iomanip>
#include <cstring>
#include <memory>


class Util {
public:
	/**
	 * hashes the input with sha512
	 * @param in    input characters
	 * @param len   length of input
	 * @param out   stream for writing the hash
	 * @param lib   dynamic library to load hash function
	 * @return 0 success, 2 library does not have required symbol
	 */
	static int hash512(const std::string &in, size_t len, std::string &out, void *lib)
	{
	        typedef int (*sha512_t)(const unsigned char *, size_t, unsigned char[], int);
	        sha512_t sha512 = reinterpret_cast<sha512_t>(dlsym(lib, "mbedtls_sha512_ret"));
	        if (!sha512) {
	                std::cerr << dlerror() << std::endl;
	                return 2;
	        }

	        unsigned char hash[64];
		auto toHash = std::make_unique<unsigned char[]>(in.size());
		std::strcpy((char*)(toHash.get()), in.c_str());

	        sha512(toHash.get(), len, hash, 0);

		std::stringstream buff;

	        buff << std::hex;

	        for (size_t i = 0; i < 64; ++i) {
	                buff << std::setfill('0') << std::setw(2) << static_cast<unsigned>(hash[i]);
	        }

	        buff << std::dec;

		out = buff.str();

	        return 0;
	}
};

#endif // UTIL_HPP
