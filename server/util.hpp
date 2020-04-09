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



/**
 * class with cryptographic functions
 * TODO: add checks
 */
class Util {
private:
	Util() {}
public:
	/**
	 * prints an unsigned char as hexadecimal to std::cout
	 * @param array		array of unsigned chars
	 * @param len		length of array
	 */
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
	 * generates an X25519 keypair and writes it to @param key
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
	 * generates an ED25519 keypair and writes it to @param key
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
	 * @param secret	shared secret
	 * @param ssize		size of shared secret
	 */
	static void ecdh(EVP_PKEY *key, EVP_PKEY *peer, unsigned char *secret, size_t *ssize)
	{
	        EVP_PKEY_CTX *ctx;

	        ctx = EVP_PKEY_CTX_new(key, NULL);

	        if (EVP_PKEY_derive_init(ctx) != 1) {
	                std::cout << "ctx initialization failed" << std::endl;
	                return;
	        }

	        if (EVP_PKEY_derive_set_peer(ctx, peer) != 1) {
	                std::cout << "setting peer failed" << std::endl;
	                return;
	        }

	        if (EVP_PKEY_derive(ctx, NULL, ssize) != 1) {
	                std::cout << "getting length failed" << std::endl;
	                return;
	        }

	        std::cout << "Length of secret = " << *ssize << std::endl;

	        EVP_PKEY_derive(ctx, secret, ssize);
	        printUnsignedChar(secret, *ssize);

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

	/**
	 * derives a key from shared secret with sha512
	 * @param secret		shared secret
	 * @param ssize			length of shared secret
	 * @param key			derived key
	 * @param keylen		length of key
	 */
	static void kdf(unsigned char *secret, size_t ssize, unsigned char *key, size_t *keylen)
	{
		while (ERR_get_error() != 0) {}
		EVP_KDF *kdf;
		EVP_KDF_CTX *kctx = NULL;
		unsigned char derived[64];
		OSSL_PARAM params[5], *p = params;


		kdf = EVP_KDF_fetch(NULL, "hkdf", NULL);
		if (kdf == NULL) {
			std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;
			return;
		}

		kctx = EVP_KDF_CTX_new(kdf);

		if (kctx == NULL) {
			std::cout << ERR_error_string(ERR_get_error(), NULL) << std::endl;
			return;
		}

		// for X25519 it is sequence of 32 0xFF bytes
		// for X448 it is sequence of 57 0xFF bytes
		unsigned char FKM[32 + ssize];
		std::memset(FKM, 0xFF, 32);
		std::memcpy(&FKM[32], secret, ssize);

		// salt is zero-filled byte sequence with same length as hash output
		unsigned char salt[64];
		std::memset(salt, 0, 64);
		// not particularly nice
		*p++ = OSSL_PARAM_construct_utf8_string("digest", const_cast<char *>("sha512"), static_cast<size_t>(7));
		*p++ = OSSL_PARAM_construct_octet_string("salt", salt, static_cast<size_t>(64));
		*p++ = OSSL_PARAM_construct_octet_string("key", FKM, ssize + 32);
		*p++ = OSSL_PARAM_construct_octet_string("info", (void *)("yolo"), static_cast<size_t>(4));
		*p = OSSL_PARAM_construct_end();

		EVP_KDF_CTX_set_params(kctx, params);

		EVP_KDF_derive(kctx, key, 64);

		EVP_KDF_free(kdf);
		EVP_KDF_CTX_free(kctx);
	}

	/**
	 * encrypts text using aes-256 in cbc mode
	 * @param plain		plaintext to be encrypted
	 * @param plen		length of the plaintext
	 * @param key		key used for encryption, must be 32 bytes long
	 * @param iv		initialization vector, must be 16 bytes long
	 * @param ciphertext	buffer where the ciphertext is written
	 * @return length of ciphertext
	 */
	static int aes256encrypt(unsigned char *plain, size_t plen, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
	{
		int len = plen;
		int clen = 0;
		EVP_CIPHER_CTX *ctx;

		ctx = EVP_CIPHER_CTX_new();

		EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

		EVP_EncryptUpdate(ctx, ciphertext, &len, plain, plen);
		clen = len;

		EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
		clen += len;

		EVP_CIPHER_CTX_free(ctx);
		return clen;
	}

	/**
	 * decrypts ciphertext from aes-256 in cbc mode
	 * @param ciphertext	encrypted text
	 * @param clen		length of ciphertext
	 * @param key		key used for decryption, must be 32 bytes long
	 * @param iv		initialization vector, must be 16 bytes long
	 * @param plain		buffer, where the decrypted text is written
	 * @return length of decrypted plaintext
	 */
	static int aes256decrypt(unsigned char *ciphertext, size_t clen, unsigned char *key, unsigned char *iv, unsigned char *plain)
	{
		int len;
		int plen;
		EVP_CIPHER_CTX *ctx;

		ctx = EVP_CIPHER_CTX_new();

		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

		EVP_DecryptUpdate(ctx, plain, &len, ciphertext, clen);
		plen = len;

		EVP_DecryptFinal_ex(ctx, plain + len, &len);
		plen += len;

		EVP_CIPHER_CTX_free(ctx);
		return plen;
	}
};

#endif // UTIL_HPP
