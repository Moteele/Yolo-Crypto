#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <dlfcn.h>
#include <iomanip>
#include <cstring>
#include <memory>
#include <vector>
#include <cstdint>

#include "../libs/include/openssl/sha.h"
#include "../libs/include/openssl/evp.h"
#include "../libs/include/openssl/kdf.h"
#include "../libs/include/openssl/pem.h"
#include "../libs/include/openssl/err.h"



class Util {
private:
	Util() {}
public:
	static void printUnsignedChar(unsigned char *array, size_t len)
	{
		std::cout << std::hex;
		for (size_t i = 0; i < len; ++i) {
			std::cout << std::setfill('0') << std::setw(2) << static_cast<unsigned>(array[i]);
		}

		std::cout << std::dec << std::endl;
	}

	/**
	 * hashes the input with sha512
	 * @param in    input string
	 * @param len   length of input
	 * @param out   output string
	 * @return 0 success, 2 library does not have required symbol
	 */
	static int hash512(const std::string &in, size_t len, std::string &out)
	{
	        unsigned char hash[64];
		std::vector<unsigned char> toHash(in.begin(), in.end());

		SHA512(&toHash[0], len, hash);
	        //mbedtls_sha512_ret(&toHash[0], in.size(), hash, 0);

		std::stringstream buff;

	        buff << std::hex;

	        for (size_t i = 0; i < 64; ++i) {
	                buff << std::setfill('0') << std::setw(2) << static_cast<unsigned>(hash[i]);
	        }

	        buff << std::dec;

		out = buff.str();

	        return 0;
	}

	/**
	 * hashes the input with sha512
	 * @param in    input string
	 * @param len   length of input
	 * @param out   output string
	 * @return 0 success, 2 library does not have required symbol
	 */
	static int hash512(const unsigned char *in, size_t len, unsigned char *out)
	{
	        SHA512(in, len, out);
	        return 0;
	}

	static void printKeys(EVP_PKEY *key)
	{
		PEM_write_PUBKEY(stdout, key);
		PEM_write_PrivateKey(stdout, key, NULL, NULL, 0, NULL, NULL);
	}

	/**
	 * generates a X25519 keypair and writes it to @param key
	 * @param key		EVP_PKEY ** structure
	 */
	static void genKeyX25519(EVP_PKEY **key)
	{
		*key = EVP_PKEY_new();
		EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
		EVP_PKEY_keygen_init(kctx);
		EVP_PKEY_keygen(kctx, key);

		EVP_PKEY_CTX_free(kctx);
		std::cout << "end of generation" << std::endl;
	}

	/**
	 * generates a ED25519 keypair and writes it to @param key
	 * @param key		EVP_PKEY ** structure
	 */
	static void genKeyED25519(EVP_PKEY **key)
	{
		*key = EVP_PKEY_new();
		EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);
		EVP_PKEY_keygen_init(kctx);
		EVP_PKEY_keygen(kctx, key);

		EVP_PKEY_CTX_free(kctx);
		std::cout << "end of generation" << std::endl;
	}

	/**
	 * TODO: add error return codes
	 * calculates shared secret from two X25519 keys.
	 * @param key		complete keypair
	 * @param peer		public key of the other side
	 */
	static void ecdh(EVP_PKEY *key, EVP_PKEY *peer)
	{
		EVP_PKEY_CTX *ctx;
		unsigned char secret[1024];
		size_t len = 0;

		ctx = EVP_PKEY_CTX_new(key, NULL);

		if (EVP_PKEY_derive_init(ctx) != 1) {
			std::cout << "ctx initialization failed" << std::endl;
			return;
		}

		if (EVP_PKEY_derive_set_peer(ctx, peer) != 1) {
			std::cout << "setting peer failed" << std::endl;
			return;
		}

		if (EVP_PKEY_derive(ctx, NULL, &len) != 1) {
			std::cout << "getting length failed" << std::endl;
			return;
		}

		std::cout << "Length of secret = " << len << std::endl;

		EVP_PKEY_derive(ctx, secret, &len);

		printUnsignedChar(secret, len);

		EVP_PKEY_CTX_free(ctx);
	}

	/**
	 * TODO: add error return codes
	 * signs a message with give ED25519 key
	 * @param key		key used for signing
	 * @param tbs		message to be signed
	 * @param tbslen	length of the message to be signed
	 * @param sig		signature of the message
	 * @param siglen	length of thesignature
	 */
	static void sign(EVP_PKEY *key, unsigned char *tbs, size_t tbslen, unsigned char *sig, size_t *siglen)
	{
		// clear errors
		while (ERR_get_error() != 0) {}

		printKeys(key);
		EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

		if (EVP_DigestSignInit(mdctx, NULL, NULL, NULL, key) != 1) {
			std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;
			return;
		}

		EVP_DigestSign(mdctx, sig, siglen, tbs, tbslen);

		std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;


		std::cout << "SIGNATURE:" << std::endl;
		printUnsignedChar(sig, *siglen);

		EVP_MD_CTX_free(mdctx);
	}

	/**
	 * TODO: add error return codes
	 * verifies an ED25519 signature of a message
	 * @param key		key used for checking the signature
	 * @param tbv		message to be verified
	 * @param tbvlen	length of message to be verified
	 * @param sig		signature of the message
	 * @param siglen	length of the signature
	 */
	static void verify(EVP_PKEY *key, unsigned char *tbv, size_t tbvlen, unsigned char *sig, size_t *siglen)
	{
		// clear errors
		while (ERR_get_error() != 0) {}

		printKeys(key);
		EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

		if (EVP_DigestVerifyInit(mdctx, NULL, NULL, NULL, key) != 1) {
			std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;
			return;
		}

		int r = EVP_DigestVerify(mdctx, sig, *siglen, tbv, tbvlen);

		std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;

		if (r == 1) {
			std::cout << "verification successfull!" << std::endl;
		} else {
			std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;
		}

		EVP_MD_CTX_free(mdctx);
	}
};

#endif // UTIL_HPP
