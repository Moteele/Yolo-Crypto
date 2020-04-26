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

#include "key.hpp"

#include "../libs/openssl_internals/curve25519.h"



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
	static void printUnsignedChar(unsigned char *array, size_t len);

	/**
	 * converts hexadecimal string to unsigned char
	 * eg.: "ab10d140" -> {'ab', '10', 'd1', '40'}
	 * @param str		string to be converted
	 * @param out		output array
	 */
	static void stringToUnsignedChar(const std::string &str, unsigned char *out);

	/**
	 * hashes the input with sha512
	 * @param in    input string
	 * @param len   length of input
	 * @param out   output string
	 * @return 0 success, 2 library does not have required symbol
	 */
	static int hash512(const std::string &in, size_t len, std::string &out);

	/**
	 * hashes the input with sha512
	 * @param in    input string
	 * @param len   length of input
	 * @param out   output string
	 * @return 0 success
	 */
	static int hash512(const unsigned char *in, size_t len, unsigned char *out);

	/**
	 * prints keypair in PEM format to stdout
	 * @param key		keypair to be printed
	 */
	static void printKeys(EVP_PKEY *key);

	/**
	 * generates an X25519 keypair and writes it to @param key
	 * @param key		EVP_PKEY ** structure
	 */
	static void genKeyX25519(EVP_PKEY **key);

	/**
	 * generates an ED25519 keypair and writes it to @param key
	 * @param key		EVP_PKEY ** structure
	 */
	static void genKeyED25519(EVP_PKEY **key);

	/**
	 * TODO: add error return codes
	 * calculates shared secret from two X25519 keys.
	 * @param key		complete keypair
	 * @param peer		public key of the other side
	 * @param secret	shared secret
	 * @param ssize		size of shared secret
	 */
	static int ecdh(EVP_PKEY *key, EVP_PKEY *peer, unsigned char *secret, size_t *ssize);

	/**
	 * TODO: add error return codes
	 * calculates shared secret from two X25519 keys.
	 * @param key		complete keypair
	 * @param peer		public key of the other side
	 * @param secret	shared secret
	 * @param ssize		size of shared secret
	 */
	static int ecdh(Key &key, Key &peer, unsigned char *secret, size_t *ssize);

	/**
	 * General KDF
	 * derives a key from shared secret with sha512, key is 64 bytes long
	 * @param secret		shared secret
	 * @param ssize			length of shared secret
	 * @param key			derived key
	 * @param keylen		length of key
	 * @param salt			salt to be used
	 * @param salt_size		length of salt
	 * @return 0 on success
	 */
	static int kdf(unsigned char *secret, size_t ssize, unsigned char *key, size_t *keylen, unsigned char *salt, size_t salt_size);
	/**
	 * x3DH KDF
	 * derives a key from shared secret with sha512, key is 64 bytes long
	 * @param secret		shared secret
	 * @param ssize			length of shared secret
	 * @param key			derived key
	 * @param keylen		length of key
	 * @return 0 on success
	 */
	static int kdf(unsigned char *secret, size_t ssize, unsigned char *key, size_t *keylen);

	/**
	 * encrypts text using aes-256 in cbc mode
	 * @param plain		plaintext to be encrypted
	 * @param plen		length of the plaintext
	 * @param key		key used for encryption, must be 32 bytes long
	 * @param iv		initialization vector, must be 16 bytes long
	 * @param ciphertext	buffer where the ciphertext is written
	 * @param padding	1 enable padding, 0 disable padding
	 * @return length of ciphertext
	 */
	static int aes256encrypt(unsigned char *plain, size_t plen, unsigned char *key, unsigned char *iv, unsigned char *ciphertext, int pad = 1);

	/**
	 * decrypts ciphertext from aes-256 in cbc mode
	 * @param ciphertext	encrypted text
	 * @param clen		length of ciphertext
	 * @param key		key used for decryption, must be 32 bytes long
	 * @param iv		initialization vector, must be 16 bytes long
	 * @param plain		buffer, where the decrypted text is written
	 * @param padding	1 enable padding, 0 disable padding
	 * @return length of decrypted plaintext
	 */
	static int aes256decrypt(unsigned char *ciphertext, size_t clen, unsigned char *key, unsigned char *iv, unsigned char *plain, int pad = 1);

	/**
	 * converts montgomery u coordinate to edwards y coordinate
	 * @param out		encoded y coordinate
	 * @param in		encoded u coordinate
	 */
	static void convert_mont(unsigned char *out, const unsigned char *in);


	/**
	 * creates edwards25519 keypair from curve25519 private key
	 * @param orig_pvt		curve25519 private key
	 * @param new_pvt		edwards25519 private key
	 * @param new_pub		edwards25519 public key
	 */
	static void calculate_key_pair(unsigned char *orig_pvt, unsigned char *new_pvt, unsigned char *new_pub);

	/**
	 * creates a signature from curve25519 private key and message
	 * @param priv		private key used for signing
	 * @param message	message to be signed
	 * @param mlen		length of message
	 * @param random	64 bytes of random data
	 * @param sig		signature, 64 bytes long
	 */
	static void xeddsa_sign(unsigned char *priv, const unsigned char *message, size_t mlen, unsigned char *random, unsigned char *sig);

	/**
	 * creates a signature from curve25519 private key and message
	 * @param priv		private key used for signing
	 * @param message	message to be signed
	 * @param mlen		length of message
	 * @param random	64 bytes of random data
	 * @param sig		signature, 64 bytes long
	 */
	static void xeddsa_sign(Key &priv, const unsigned char *message, size_t mlen, unsigned char *random, unsigned char *sig);

	/**
	 * verifies a signature given the message and public key
	 * @param pub		public key for checking
	 * @param message	message
	 * @param mlen		length of message
	 * @param sig		signature ofmessage
	 * @return 0 on success
	 */
	static int xeddsa_verify(unsigned char *pub, const unsigned char *message, size_t mlen, unsigned char *sig);

	/**
	 * verifies a signature given the message and public key
	 * @param pub		public key for checking
	 * @param message	message
	 * @param mlen		length of message
	 * @param sig		signature ofmessage
	 * @return 0 on success
	 */
	static int xeddsa_verify(Key &pub, const unsigned char *message, size_t mlen, unsigned char *sig);
};


struct keyPair {
    unsigned char key1[32];
    unsigned char key2[32];
};

class Ratchet {
    public:
	Key *DHs;
	EVP_PKEY *DHr;
	unsigned char *RK = { nullptr };
	unsigned char *CKs[32] = { nullptr };
	unsigned char *CKr[32] = { nullptr };
	int Ns = 0, Nr = 0;
	int PN = 0;
	std::vector<unsigned char[32]> MKSKIPPED;

	void InitA(unsigned char* SK, unsigned char* BpubKey);

	void InitB(unsigned char* SK, Key *BkeyPair);

	keyPair kdf_rk(unsigned char* RK, unsigned char* dh_out);

	keyPair kdf_ck(unsigned char* CK);
};

#endif // UTIL_HPP
