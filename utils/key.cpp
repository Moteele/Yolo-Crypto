#include <vector>
#include <iomanip>
#include <iostream>
#include <openssl/kdf.h>
#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ecdh.h>

#include "key.hpp"
#include "util.hpp"


Key::Key(unsigned char *key, bool pub)
{
	if (pub) {
		key_ = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, key, 32);
	} else {
		key_ = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, key, 32);
	}

	privKey = getPrivateKey_();
	pubKey = getPublicKey_();
}

Key::~Key() { EVP_PKEY_free(key_); }

void Key::generate()
{
	EVP_PKEY_CTX *ctx_ = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
	EVP_PKEY_keygen_init(ctx_);
	EVP_PKEY_keygen(ctx_, &key_);
	EVP_PKEY_CTX_free(ctx_);

	privKey = getPrivateKey_();
	pubKey = getPublicKey_();
}

void Key::clear() { EVP_PKEY_free(key_); }

void Key::setPublic(unsigned char *pub)
{
	this->clear();
	key_ = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, pub, 32);

	pubKey = getPublicKey_();
}

void Key::setPrivate(unsigned char *priv)
{
	this->clear();
	key_ = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, priv, 32);

	privKey = getPrivateKey_();
	pubKey = getPublicKey_();
}

std::vector<uint8_t> Key::getPrivateKey_()
{
	size_t len = 32;
	std::vector<unsigned char> out(32);
	EVP_PKEY_get_raw_private_key(key_, &out[0], &len);
	return out;
}

std::vector<uint8_t> Key::getPublicKey_()
{
	size_t len = 32;
	std::vector<uint8_t> out(32);
	EVP_PKEY_get_raw_public_key(key_, &out[0], &len);
	return out;
}

std::vector<uint8_t> Key::getPrivateKey() {return privKey;}
std::vector<uint8_t> Key::getPublicKey() {return pubKey;}

EVP_PKEY *Key::getPkey() { return key_; }

void Key::printPrivate()
{
	//Util::printUnsignedChar(&getPrivateKey()[0], 32);
}

void Key::printPublic()
{
	//Util::printUnsignedChar(&getPublicKey()[0], 32);
}
