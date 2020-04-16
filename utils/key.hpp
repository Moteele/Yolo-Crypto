#include <vector>
#include <iomanip>
#include <iostream>
#include <openssl/kdf.h>
#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ecdh.h>




/**
 * curve25519 key wrapper class
 */
class Key
{
private:
	EVP_PKEY *key = NULL;
	EVP_PKEY_CTX *ctx = NULL;
public:
	Key()
	{
		key = EVP_PKEY_new();
		ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
		EVP_PKEY_keygen_init(ctx);
		EVP_PKEY_keygen(ctx, &key);
	}

	~Key()
	{
		EVP_PKEY_free(key);
		EVP_PKEY_CTX_free(ctx);
	}

	std::vector<uint8_t> getPrivateKey()
	{
		size_t len = 32;
		std::vector<unsigned char> out(32);
		EVP_PKEY_get_raw_private_key(key, &out[0], &len);
		return out;
	}

	std::vector<uint8_t> getPublicKey()
	{
		size_t len = 32;
		std::vector<uint8_t> out(32);
		EVP_PKEY_get_raw_public_key(key, &out[0], &len);
		return out;
	}

	EVP_PKEY *getPkey() { return key; }

	void printPrivate()
	{
		std::cout << std::hex;
        	for (auto &el : getPrivateKey()) {
        	        std::cout << std::setfill('0') << std::setw(2) << static_cast<unsigned>(el);
        	}
        	std::cout << std::dec << std::endl;
	}

	void printPublic()
	{
		std::cout << std::hex;
        	for (auto &el : getPublicKey()) {
        	        std::cout << std::setfill('0') << std::setw(2) << static_cast<unsigned>(el);
        	}
        	std::cout << std::dec << std::endl;
	}
};
