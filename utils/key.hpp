#ifndef KEY_HPP
#define KEY_HPP

#include <vector>
#include <openssl/evp.h>




/**
 * curve25519 key wrapper class
 */
class Key
{
private:
	EVP_PKEY *key_ = NULL;
public:
	/**
	 * default constructor
	 */
	Key() = default;

	/**
	 * constructor, takes public/private key 32 bytes long and sets it
	 * @param key		key to be set
	 * @param pub		which key is to be set - true = PUBLIC, false = PRIVATE
	 */
	Key(unsigned char *key, bool pub);

	/**
	 * destructor, frees the internal key structure
	 */
	~Key();

	/**
	 * generates a new keypair
	 */
	void generate();

	/**
	 * clears the internal key structure
	 */
	void clear();

	/**
	 * CLEARS THE INTERNAL KEY STRUCTURE and sets new public key
	 * @param pub		public key to be set
	 */
	void setPublic(unsigned char *pub);

	/**
	 * CLEARS THE INTERNAL KEY STRUCTURE and sets new private key,
	 * new public key is generated automatically
	 * @param priv		private key to be set
	 */
	void setPrivate(unsigned char *priv);

	/**
	 * returns the private key as a sequence of 32 bytes
	 * @return vector with private key
	 */
	std::vector<uint8_t> getPrivateKey();

	/**
	 * returns the public key as a sequence of 32 bytes
	 * @return vector with public key
	 */
	std::vector<uint8_t> getPublicKey();

	/**
	 * returns the EVP_PKEY structure that contains the keypair
	 * @return keypair
	 */
	EVP_PKEY *getPkey();

	/**
	 * prints the private key
	 */
	void printPrivate();

	/**
	 * prints the public key
	 */
	void printPublic();
};

#endif // KEY_HPP
